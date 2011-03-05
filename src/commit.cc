#include "commit.h"
#include "repository.h"
#include "tree.h"
#include <time.h>

#define ID_PROPERTY String::NewSymbol("id")
#define MESSAGE_PROPERTY String::NewSymbol("message")
#define TIME_PROPERTY String::NewSymbol("time")
#define AUTHOR_PROPERTY String::NewSymbol("author")
#define COMMITTER_PROPERTY String::NewSymbol("committer")
#define TREE_PROPERTY String::NewSymbol("tree")

#define SIG_TIME_PROPERTY String::NewSymbol("time")
#define SIG_EMAIL_PROPERTY String::NewSymbol("email")
#define SIG_NAME_PROPERTY String::NewSymbol("name")

#define CHECK_PROPERTY(PROPNAME)											\
	if(args.This()->Get(PROPNAME)->IsUndefined()) 							\
		THROW_ERROR("Commit property " #PROPNAME " is required.");

#define GET_SIGNATURE_PROPERTY(PROPNAME, VAR)								\
	if(!args.This()->Get(PROPNAME)->IsObject()) 							\
		THROW_ERROR("Property " #VAR " should be an object.");				\
	Handle<Object> VAR ## Obj = Handle<Object>::Cast(						\
			args.This()->Get(PROPNAME));									\
	Handle<Date> VAR ## ObjDate = Handle<Date>::Cast(						\
			VAR ## Obj->Get(SIG_TIME_PROPERTY));							\
	if(!VAR ## ObjDate->IsDate())											\
		THROW_ERROR(#VAR " date is invalid");								\
	time_t VAR ## Time = VAR ## ObjDate->NumberValue();						\
	Handle<String> VAR ## ObjName = 										\
			VAR ## Obj->Get(SIG_NAME_PROPERTY)->ToString();					\
	if(!VAR ## ObjName->Length())											\
		THROW_ERROR(#VAR " name cannot be empty.");							\
	String::Utf8Value VAR ## Name(VAR ## ObjName);							\
	Handle<String> VAR ## ObjEmail = 										\
			VAR ## Obj->Get(SIG_EMAIL_PROPERTY)->ToString();				\
	if(!VAR ## ObjEmail->Length())											\
		THROW_ERROR(#VAR " email cannot be empty.");						\
	String::Utf8Value VAR ## Email(VAR ## ObjEmail);						\
	git_signature *VAR = git_signature_new(*VAR ## Name,					\
			*VAR ## Email, VAR ## Time, 0);									\
	if(VAR == NULL) THROW_ERROR("Couldn't create signature.");

namespace gitteh {

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "setTree", SetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "addParent", AddParent);
	NODE_SET_PROTOTYPE_METHOD(t, "getParent", GetParent);
	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();

	// Setup some basic info about this commit.
	const git_oid *commitId = git_commit_id(commit->commit_);

	Handle<Object> jsObj = args.This();
	if(commitId) {
		const char* oidStr = git_oid_allocfmt(commitId);

		jsObj->Set(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));
		const char* message = git_commit_message(commit->commit_);
		jsObj->Set(MESSAGE_PROPERTY, String::New(message));

		const git_signature *author;
		author = git_commit_author(commit->commit_);
		if(author) {
			CREATE_PERSON_OBJ(authorObj, author);
			jsObj->Set(AUTHOR_PROPERTY, authorObj);
		}

		const git_signature *committer;
		committer = git_commit_committer(commit->commit_);
		if(committer) {
			CREATE_PERSON_OBJ(committerObj, committer);
			jsObj->Set(COMMITTER_PROPERTY, committerObj);
		}

		commit->parentCount_ = git_commit_parentcount(commit->commit_);

		jsObj->Set(String::New("parentCount"), Integer::New(commit->parentCount_), (PropertyAttribute)(ReadOnly | DontDelete));
	}
	else {
		// This is a new commit.
		jsObj->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(MESSAGE_PROPERTY, Null());
		jsObj->Set(AUTHOR_PROPERTY, Null());
		jsObj->Set(COMMITTER_PROPERTY, Null());
		commit->parentCount_ = 0;
		jsObj->Set(String::New("parentCount"), Integer::New(0), (PropertyAttribute)(ReadOnly | DontDelete));
	}

	commit->Wrap(args.This());
	return args.This();
}

Handle<Value> Commit::GetTree(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	const git_tree *tree = git_commit_tree(commit->commit_);
	if(tree == NULL) {
		return scope.Close(Null());
	}

	Tree *treeObject = commit->repository_->wrapTree(const_cast<git_tree*>(tree));
	return scope.Close(treeObject->handle_);
}

Handle<Value> Commit::SetTree(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	git_tree *tree;
	if(args[0]->IsString()) {
		git_oid treeId;
		int res = git_oid_mkstr(&treeId, *String::Utf8Value(args[0]));
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Id is invalid", res);
		}

		res = git_tree_lookup(&tree, commit->repository_->repo_, &treeId);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error locating tree", res);
		}
	}
	else if(Tree::constructor_template->HasInstance(args[0])) {
		Tree *treeObj = ObjectWrap::Unwrap<Tree>(Handle<Object>::Cast(args[0]));
		tree = treeObj->tree_;
	}

	git_commit_set_tree(commit->commit_, tree);
}

Handle<Value> Commit::GetParent(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	REQ_ARGS(1);
	REQ_INT_ARG(0, index);

	if(index >= commit->parentCount_) {
		return ThrowException(Exception::Error(String::New("Parent commit index is out of bounds.")));
	}

	git_commit *parent = git_commit_parent(commit->commit_, index);
	Commit *parentObject = commit->repository_->wrapCommit(parent);
	return scope.Close(parentObject->handle_);
}

Handle<Value> Commit::AddParent(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	REQ_ARGS(1);

	git_commit *parentCommit;
	if(args[0]->IsString()) {
		git_oid commitId;
		int res = git_oid_mkstr(&commitId, *String::Utf8Value(args[0]));
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Id is invalid", res);
		}

		res = git_commit_lookup(&parentCommit, commit->repository_->repo_, &commitId);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error locating commit", res);
		}
	}
	else if(constructor_template->HasInstance(args[0])) {
		Commit *otherCommit = ObjectWrap::Unwrap<Commit>(Handle<Object>::Cast(args[0]));
		parentCommit = otherCommit->commit_;
	}
	else {
		THROW_ERROR("Invalid argument.");
	}

	git_commit_add_parent(commit->commit_, parentCommit);

	args.This()->ForceSet(String::New("parentCount"), Integer::New(commit->parentCount_++), (PropertyAttribute)(ReadOnly | DontDelete));

	return scope.Close(Undefined());
}

Handle<Value> Commit::Save(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	CHECK_PROPERTY(MESSAGE_PROPERTY);
	Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
	if(message->Length() == 0) {
		THROW_ERROR("Message must not be empty.");
	}

	GET_SIGNATURE_PROPERTY(AUTHOR_PROPERTY, author);
	GET_SIGNATURE_PROPERTY(COMMITTER_PROPERTY, committer);

	git_commit_set_message(commit->commit_, *String::Utf8Value(message));
	git_commit_set_committer(commit->commit_, committer);
	git_commit_set_author(commit->commit_, author);

	int result = git_object_write((git_object *)commit->commit_);
	if(result != GIT_SUCCESS) {
		return ThrowException(Exception::Error(String::New("Failed to save commit object.")));
	}

	const git_oid *commitId = git_commit_id(commit->commit_);
	const char* oidStr = git_oid_allocfmt(commitId);
	args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

	return True();
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}

} // namespace gitteh
