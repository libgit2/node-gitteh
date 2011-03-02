#include "rev_walker.h"

Persistent<FunctionTemplate> RevWalker::constructor_template;

void RevWalker::Init(Handle<Object> target) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("RevWalker"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
	NODE_SET_PROTOTYPE_METHOD(t, "hide", Hide);
	NODE_SET_PROTOTYPE_METHOD(t, "next", Next);
}

Handle<Value> RevWalker::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(2);
	REQ_EXT_ARG(0, theWalker);
	REQ_EXT_ARG(1, theRepo);

	RevWalker *walker = new RevWalker();
	walker->walker_ = static_cast<git_revwalk *>(theWalker->Value());
	walker->repo_ = static_cast<Repository *>(theRepo->Value());

	walker->Wrap(args.This());
	return args.This();
}

Handle<Value> RevWalker::Push(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit;
	if(args[0]->IsString()) {
		REQ_OID_ARG(0, commitOid);
		int result = git_commit_lookup(&commit, walker->repo_, &commitOid);

		if(result != GIT_SUCCESS) {
			return ThrowException(Exception::Error(String::New("Commit not found.")));
		}
	}
	else {
		// Commit object.
		return ThrowException(Exception::Error(String::New("Passing commit object is not supported yet.")));
	}

	// Get the commit for this oid.
	git_revwalk_push(walker->walker_, commit);

	return Undefined();
}

Handle<Value> RevWalker::Hide(const Arguments& args) {
	HandleScope scope;

	return Undefined();
}

Handle<Value> RevWalker::Next(const Arguments& args) {
	HandleScope scope;

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit = git_revwalk_next(walker->walker_);

	if(commit == NULL) {
		return Null();
	}

	Commit *commitObject = walker->repo_->wrapCommit(commit);

	return commitObject->handle_;
}

RevWalker::~RevWalker() {
	git_revwalk_free(walker_);
}
