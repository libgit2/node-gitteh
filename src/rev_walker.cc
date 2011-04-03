/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "rev_walker.h"
#include "commit.h"
#include "repository.h"
#include "gitobjectwrap.h"

namespace gitteh {

static Persistent<String> revwalker_class_symbol;

struct walker_request {
	Persistent<Function> callback;
	RevWalker *walker;
	git_commit *commit;
	Commit *commitObject;
	std::string *id;
	int error;
};

struct sort_request {
	Persistent<Function> callback;
	RevWalker *walker;
	int sorting;
	int error;
};

struct reset_request {
	Persistent<Function> callback;
	RevWalker *walker;
};

Persistent<FunctionTemplate> RevWalker::constructor_template;

void RevWalker::Init(Handle<Object> target) {
	HandleScope scope;

	revwalker_class_symbol = NODE_PSYMBOL("RevisionWalker");

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(revwalker_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
	NODE_SET_PROTOTYPE_METHOD(t, "hide", Hide);
	NODE_SET_PROTOTYPE_METHOD(t, "next", Next);
	NODE_SET_PROTOTYPE_METHOD(t, "sort", Sort);
	NODE_SET_PROTOTYPE_METHOD(t, "reset", Reset);

	NODE_DEFINE_CONSTANT(target, GIT_SORT_NONE);
	NODE_DEFINE_CONSTANT(target, GIT_SORT_TOPOLOGICAL);
	NODE_DEFINE_CONSTANT(target, GIT_SORT_TIME);
	NODE_DEFINE_CONSTANT(target, GIT_SORT_REVERSE);

	target->Set(revwalker_class_symbol, constructor_template->GetFunction());
}

Handle<Value> RevWalker::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theWalker);

	RevWalker *walker = new RevWalker();
	walker->walker_ = static_cast<git_revwalk *>(theWalker->Value());

	walker->Wrap(args.This());

	return args.This();
}

Handle<Value> RevWalker::Push(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	if(HAS_CALLBACK_ARG) {
		walker_request *request = new walker_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->callback = Persistent<Function>::New(callbackArg);
		request->walker = walker;

		if(Commit::constructor_template->HasInstance(args[0])) {
			request->commit = ObjectWrap::Unwrap<Commit>(Handle<Object>::Cast(args[0]))
					->commit_;

			// TODO: what would happen if the commit was GC'd when we go to the
			// EIO threadpool? I think it should be ok since we're not
			// referencing the js object, but just the underlying commit itself
			// which shouldn't get gc'd.
			// FIXME: No, with the new libgit2 changes the git_commit is freed
			// when the wrapping object is gc'd. We need to make sure that doesn't
			// happen.
			//request->commit->Ref();
		}
		else {
			request->commit = NULL;
			request->id = new std::string(*String::Utf8Value(args[0]));
		}

		walker->Ref();
		eio_custom(EIO_Push, EIO_PRI_DEFAULT, EIO_AfterPush, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_commit *commit;
		if(Commit::constructor_template->HasInstance(args[0])) {
			Handle<Object> commitArg = Handle<Object>::Cast(args[0]);
			Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
			commit = commitObject->commit_;
			//return ThrowException(Exception::Error(String::New("Passing commit object is not supported yet.")));
		}
		else {
			REQ_OID_ARG(0, commitOid);
			walker->repo_->lockRepository();
			int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);
			walker->repo_->unlockRepository();
			if(result != GIT_SUCCESS)
				THROW_GIT_ERROR("Commit not found.", result);
		}

		// Get the commit for this oid.
		walker->repo_->lockRepository();
		int res = git_revwalk_push(walker->walker_, git_commit_id(commit));
		walker->repo_->unlockRepository();
		if(res != GIT_SUCCESS)
			THROW_GIT_ERROR("Couldn't push commit onto walker.", res);

		return scope.Close(True());
	}
}

int RevWalker::EIO_Push(eio_req *req) {
	walker_request *reqData = static_cast<walker_request*>(req->data);

	git_commit *commit;
	if(reqData->commit != NULL) {
		commit = reqData->commit;
		reqData->error = GIT_SUCCESS;
	}
	else {
		reqData->walker->repo_->lockRepository();
		git_oid commitOid;
		git_oid_mkstr(&commitOid, reqData->id->c_str());
		reqData->error = git_commit_lookup(&commit, reqData->walker->repo_->repo_, &commitOid);
		reqData->walker->repo_->unlockRepository();

		delete reqData->id;

	}

	if(reqData->error == GIT_SUCCESS) {
		reqData->walker->repo_->lockRepository();
		reqData->error = git_revwalk_push(reqData->walker->walker_,
				git_commit_id(commit));
		reqData->walker->repo_->unlockRepository();
	}
	return 0;
}

int RevWalker::EIO_AfterPush(eio_req *req) {
	HandleScope scope;
	walker_request *reqData = static_cast<walker_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
	reqData->walker->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_SUCCESS) {
		Handle<Value> error = CreateGitError(String::New("Couldn't push commit."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		callbackArgs[0] = Null();
		callbackArgs[1] = True();
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

Handle<Value> RevWalker::Hide(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	if(HAS_CALLBACK_ARG) {
		walker_request *request = new walker_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->callback = Persistent<Function>::New(callbackArg);
		request->walker = walker;

		if(Commit::constructor_template->HasInstance(args[0])) {
			request->commit = ObjectWrap::Unwrap<Commit>(Handle<Object>::Cast(args[0]))
					->commit_;
		}
		else {
			request->commit = NULL;
			request->id = new std::string(*String::Utf8Value(args[0]));
		}

		walker->Ref();
		eio_custom(EIO_Hide, EIO_PRI_DEFAULT, EIO_AfterHide, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_commit *commit;
		if(Commit::constructor_template->HasInstance(args[0])) {
			Handle<Object> commitArg = Handle<Object>::Cast(args[0]);

			Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
			commit = commitObject->commit_;
		}
		else {
			REQ_OID_ARG(0, commitOid);
			int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);

			if(result != GIT_SUCCESS) {
				return ThrowException(Exception::Error(String::New("Commit not found.")));
			}
		}


		int res = git_revwalk_hide(walker->walker_, git_commit_id(commit));
		if(res != GIT_SUCCESS)
			THROW_GIT_ERROR("Couldn't hide commit.", res);
	
		return scope.Close(True());
	}
}

int RevWalker::EIO_Hide(eio_req *req) {
	walker_request *reqData = static_cast<walker_request*>(req->data);

	git_commit *commit;
	if(reqData->commit != NULL) {
		commit = reqData->commit;
		reqData->error = GIT_SUCCESS;
	}
	else {
		reqData->walker->repo_->lockRepository();
		git_oid commitOid;
		git_oid_mkstr(&commitOid, reqData->id->c_str());
		reqData->error = git_commit_lookup(&commit, reqData->walker->repo_->repo_, &commitOid);
		reqData->walker->repo_->unlockRepository();

		delete reqData->id;

	}

	if(reqData->error == GIT_SUCCESS) {
		reqData->walker->repo_->lockRepository();
		reqData->error = git_revwalk_hide(reqData->walker->walker_,
				git_commit_id(commit));
		reqData->walker->repo_->unlockRepository();
	}
	return 0;
}

int RevWalker::EIO_AfterHide(eio_req *req) {
	HandleScope scope;
	walker_request *reqData = static_cast<walker_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
	reqData->walker->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_SUCCESS) {
		Handle<Value> error = CreateGitError(String::New("Couldn't hide commit."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		callbackArgs[0] = Null();
		callbackArgs[1] = True();
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

Handle<Value> RevWalker::Next(const Arguments& args) {
	HandleScope scope;
	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	if(HAS_CALLBACK_ARG) {
		walker_request *request = new walker_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->callback = Persistent<Function>::New(callbackArg);
		request->walker = walker;

		walker->Ref();
		eio_custom(EIO_Next, EIO_PRI_DEFAULT, EIO_AfterNext, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_oid id;

		walker->repo_->lockRepository();
		int result = git_revwalk_next(&id, walker->walker_);
		walker->repo_->unlockRepository();

		if(result == GIT_EREVWALKOVER) {
			return Null();
		}

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't get next commit.", result);
		}

		git_commit *commit;
		walker->repo_->lockRepository();
		result = git_commit_lookup(&commit, walker->repo_->repo_, &id);
		walker->repo_->unlockRepository();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't get next commit.", result);
		}

		return scope.Close(walker->repo_->commitCache_->syncRequest(commit));
		//return scope.Close(walker->repo_->commitFactory_->syncRequestObject(commit)->handle_);
	}
}

int RevWalker::EIO_Next(eio_req *req) {
	walker_request *reqData = static_cast<walker_request*>(req->data);

	git_oid id;
	reqData->walker->repo_->lockRepository();
	reqData->error = git_revwalk_next(&id, reqData->walker->walker_);
	reqData->walker->repo_->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		reqData->error = git_commit_lookup(&reqData->commit,
				reqData->walker->repo_->repo_, &id);

		if(reqData->error == GIT_SUCCESS) {
			reqData->error = reqData->walker->repo_->commitCache_->asyncRequest(
					reqData->commit, &reqData->commitObject);
		}
	}

	return 0;
}

int RevWalker::EIO_AfterNext(eio_req *req) {
	HandleScope scope;
	walker_request *reqData = static_cast<walker_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->walker->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error == GIT_EREVWALKOVER) {
		callbackArgs[0] = Undefined();
		callbackArgs[1] = Null();
 		TRIGGER_CALLBACK();
 		reqData->callback.Dispose();
	}
	else if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't get next commit."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->commitObject->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(reqData->commitObject->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;

	return 0;
}

Handle<Value> RevWalker::Sort(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, sorting);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	if(HAS_CALLBACK_ARG) {
		sort_request *request = new sort_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->callback = Persistent<Function>::New(callbackArg);
		request->walker = walker;
		request->sorting = sorting;

		walker->Ref();
		eio_custom(EIO_Sort, EIO_PRI_DEFAULT, EIO_AfterSort, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {

		walker->repo_->lockRepository();
		git_revwalk_sorting(walker->walker_, sorting);
		walker->repo_->unlockRepository();

		return True();
	}
}

int RevWalker::EIO_Sort(eio_req *req) {
	sort_request *reqData = static_cast<sort_request*>(req->data);

	reqData->walker->repo_->lockRepository();
	git_revwalk_sorting(reqData->walker->walker_, reqData->sorting);
	reqData->error = GIT_SUCCESS;
	reqData->walker->repo_->unlockRepository();

	return 0;
}

int RevWalker::EIO_AfterSort(eio_req *req) {
	HandleScope scope;
	sort_request *reqData = static_cast<sort_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->walker->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't set sorting."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Undefined();
 		callbackArgs[1] = True();
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> RevWalker::Reset(const Arguments& args) {
	HandleScope scope;
	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());
	
	if(HAS_CALLBACK_ARG) {
		reset_request *request = new reset_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->callback = Persistent<Function>::New(callbackArg);
		request->walker = walker;

		walker->Ref();
		eio_custom(EIO_Reset, EIO_PRI_DEFAULT, EIO_AfterReset, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		walker->repo_->lockRepository();
		git_revwalk_reset(walker->walker_);
		walker->repo_->unlockRepository();

		return scope.Close(True());
	}
}

int RevWalker::EIO_Reset(eio_req *req) {
	reset_request *reqData = static_cast<reset_request*>(req->data);

	reqData->walker->repo_->lockRepository();
	git_revwalk_reset(reqData->walker->walker_);
	reqData->walker->repo_->unlockRepository();

	return 0;
}

int RevWalker::EIO_AfterReset(eio_req *req) {
	HandleScope scope;
	reset_request *reqData = static_cast<reset_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->walker->Unref();

	Handle<Value> callbackArgs[2];
	callbackArgs[0] = Undefined();
	callbackArgs[1] = True();

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

RevWalker::~RevWalker() {
	repo_->lockRepository();
	git_revwalk_free(walker_);
	repo_->unlockRepository();
}

} // namespace gitteh
