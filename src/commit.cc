#include "commit.h"

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getParent", GetParent);
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();

	// Setup some basic info about this commit.
	const char* oidStr = git_oid_allocfmt(git_commit_id(commit->commit_));
	args.This()->Set(String::New("id"), String::New(oidStr), ReadOnly);

	const char* message = git_commit_message(commit->commit_);
	args.This()->Set(String::New("message"), String::New(message));

	const char* shortMessage = git_commit_message_short(commit->commit_);
	args.This()->Set(String::New("shortMessage"), String::New(shortMessage), ReadOnly);

	time_t time = git_commit_time(commit->commit_);
	args.This()->Set(String::New("time"), Date::New(static_cast<double>(time)*1000));

	const git_signature *author;
	author = git_commit_author(commit->commit_);
	CREATE_PERSON_OBJ(authorObj, author);
	args.This()->Set(String::New("author"), authorObj);

	const git_signature *committer;
	committer = git_commit_committer(commit->commit_);
	CREATE_PERSON_OBJ(committerObj, committer);
	args.This()->Set(String::New("committer"), committerObj);

	commit->parentCount_ = git_commit_parentcount(commit->commit_);

	args.This()->Set(String::New("parentCount"), Integer::New(commit->parentCount_), ReadOnly);

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

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}
