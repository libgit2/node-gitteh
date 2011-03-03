#include "rev_walker.h"
#include "commit.h"
#include "repository.h"

namespace gitteh {

Persistent<FunctionTemplate> RevWalker::constructor_template;

void RevWalker::Init(Handle<Object> target) {
	HandleScope scope;

	target->Set(String::New("SORT_NONE"), Integer::New(GIT_SORT_NONE));
	target->Set(String::New("SORT_TOPOLOGICAL"), Integer::New(GIT_SORT_TOPOLOGICAL));
	target->Set(String::New("SORT_TIME"), Integer::New(GIT_SORT_TIME));
	target->Set(String::New("SORT_REVERSE"), Integer::New(GIT_SORT_REVERSE));

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("RevWalker"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
	NODE_SET_PROTOTYPE_METHOD(t, "hide", Hide);
	NODE_SET_PROTOTYPE_METHOD(t, "next", Next);
	NODE_SET_PROTOTYPE_METHOD(t, "sort", Sort);
	NODE_SET_PROTOTYPE_METHOD(t, "reset", Reset);
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
		int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);

		if(result != GIT_SUCCESS) {
			return ThrowException(Exception::Error(String::New("Commit not found.")));
		}
	}
	else {
		// Commit object.
		if(!args[0]->IsObject()) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Handle<Object> commitArg = Handle<Object>::Cast(args[0]);
		if(!Commit::constructor_template->HasInstance(commitArg)) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
		commit = commitObject->commit_;
		//return ThrowException(Exception::Error(String::New("Passing commit object is not supported yet.")));
	}

	// Get the commit for this oid.
	git_revwalk_push(walker->walker_, commit);

	return Undefined();
}

Handle<Value> RevWalker::Hide(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit;
	if(args[0]->IsString()) {
		REQ_OID_ARG(0, commitOid);
		int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);

		if(result != GIT_SUCCESS) {
			return ThrowException(Exception::Error(String::New("Commit not found.")));
		}
	}
	else {
		// Commit object.
		if(!args[0]->IsObject()) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Handle<Object> commitArg = Handle<Object>::Cast(args[0]);
		if(!Commit::constructor_template->HasInstance(commitArg)) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
		commit = commitObject->commit_;
		//return ThrowException(Exception::Error(String::New("Passing commit object is not supported yet.")));
	}
	
	git_revwalk_hide(walker->walker_, commit);

	return Undefined();
}

Handle<Value> RevWalker::Next(const Arguments& args) {
	HandleScope scope;

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit;
	git_revwalk_next(&commit, walker->walker_);

	if(commit == NULL) {
		return Null();
	}

	Commit *commitObject = walker->repo_->wrapCommit(commit);
	return commitObject->handle_;
}

Handle<Value> RevWalker::Sort(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, sorting);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_revwalk_sorting(walker->walker_, sorting);
}

Handle<Value> RevWalker::Reset(const Arguments& args) {
	HandleScope scope;
	
	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());
	git_revwalk_reset(walker->walker_);

	return Undefined();
}

RevWalker::~RevWalker() {
	git_revwalk_free(walker_);
}

} // namespace gitteh
