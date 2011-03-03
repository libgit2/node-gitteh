#include "commit.h"
#include "repository.h"
#include "tree.h"
#include <time.h>

namespace gitteh {

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getParent", GetParent);
	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();

	commit->syncWithUnderlying(args.This());

	commit->Wrap(args.This());
	return args.This();
}

Handle<Value> Commit::GetTree(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	const git_tree *tree = git_commit_tree(commit->commit_);

	Tree *treeObject = commit->repository_->wrapTree(const_cast<git_tree*>(tree));
	return treeObject->handle_;
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
	return parentObject->handle_;
}

Handle<Value> Commit::Save(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	git_commit_set_message(commit->commit_, *String::Utf8Value(args.This()->Get(String::New("message"))));

	int result = git_object_write((git_object *)commit->commit_);
	if(result != GIT_SUCCESS) {
		return ThrowException(Exception::Error(String::New("Failed to save commit object.")));
	}

	// Reload the commit object now.
	commit->syncWithUnderlying(args.This());

	return True();
}

void Commit::syncWithUnderlying(Handle<Object> jsObj) {
	// Setup some basic info about this commit.
	const git_oid *commitId = git_commit_id(commit_);

	if(commitId) {
		const char* oidStr = git_oid_allocfmt(commitId);

		jsObj->Set(String::New("id"), String::New(oidStr), ReadOnly);
		const char* message = git_commit_message(commit_);
		jsObj->Set(String::New("message"), String::New(message));

		const char* shortMessage = git_commit_message_short(commit_);
		jsObj->Set(String::New("shortMessage"), String::New(shortMessage), ReadOnly);

		time_t time = git_commit_time(commit_);
		jsObj->Set(String::New("time"), Date::New(static_cast<double>(time)*1000));

		const git_signature *author;
		author = git_commit_author(commit_);
		if(author) {
			CREATE_PERSON_OBJ(authorObj, author);
			jsObj->Set(String::New("author"), authorObj);
		}

		const git_signature *committer;
		committer = git_commit_committer(commit_);
		if(committer) {
			CREATE_PERSON_OBJ(committerObj, committer);
			jsObj->Set(String::New("committer"), committerObj);
		}

		parentCount_ = git_commit_parentcount(commit_);

		jsObj->Set(String::New("parentCount"), Integer::New(parentCount_), ReadOnly);
	}
	else {
		// This is a new commit.
		jsObj->Set(String::New("id"), Null(), ReadOnly);
		jsObj->Set(String::New("message"), String::New(""));
		jsObj->Set(String::New("shortMessage"), String::New(""), ReadOnly);
		jsObj->Set(String::New("time"), Date::New(static_cast<double>(time(NULL))*1000));
		jsObj->Set(String::New("author"), Null());
		jsObj->Set(String::New("committer"), Null());
		jsObj->Set(String::New("parentCount"), Integer::New(0), ReadOnly);
	}
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}

} // namespace gitteh
