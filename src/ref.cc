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

#include "ref.h"
#include "repository.h"

static Persistent<String> ref_class_symbol;
static Persistent<String> name_symbol;
static Persistent<String> type_symbol;
static Persistent<String> target_symbol;

#define CHECK_DELETED()														\
	ref->lock();															\
	if(ref->deleted_) { 													\
		ref->unlock();														\
		THROW_ERROR("Reference is deleted.");								\
	}

#define IF_ASYNC_CHECK_ISNT_DELETED()										\
	reqData->ref->lock();													\
	bool deleted = reqData->ref->deleted_;									\
	if(reqData->ref->deleted_) { 											\
		reqData->error = GIT_EINVALIDREFSTATE;								\
		reqData->ref->unlock();												\
	}																		\
	else {

namespace gitteh {

struct ref_data {
	std::string *name;
	git_rtype type;
	std::string *target;
};

struct ref_request {
	Persistent<Function> callback;
	Reference *ref;
	std::string *name;
	git_oid id;
	int error;
};

struct resolve_request {
	Persistent<Function> callback;
	Reference *ref;
	git_reference *resolved;
	Reference *resolvedObj;
	int error;
};

struct target_request {
	Persistent<Function> callback;
	Reference *ref;
	std::string *target;
	int error;
};

Persistent<FunctionTemplate> Reference::constructor_template;

Reference::Reference(git_reference *ref) {
	CREATE_MUTEX(lock_);
	deleted_ = false;
	ref_ = ref;
}

void Reference::lock() {
	LOCK_MUTEX(lock_);
}

void Reference::unlock() {
	UNLOCK_MUTEX(lock_);
}

void Reference::Init(Handle<Object> target) {
	HandleScope scope;

	ref_class_symbol = NODE_PSYMBOL("Reference");
	name_symbol = NODE_PSYMBOL("name");
	type_symbol = NODE_PSYMBOL("type");
	target_symbol = NODE_PSYMBOL("target");

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Reference"));
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "rename", Rename);
	NODE_SET_PROTOTYPE_METHOD(t, "delete", Delete);
	NODE_SET_PROTOTYPE_METHOD(t, "resolve", Resolve);
	NODE_SET_PROTOTYPE_METHOD(t, "setTarget", SetTarget);

	NODE_DEFINE_CONSTANT(target, GIT_REF_OID);
	NODE_DEFINE_CONSTANT(target, GIT_REF_SYMBOLIC);
	NODE_DEFINE_CONSTANT(target, GIT_REF_PACKED);
	NODE_DEFINE_CONSTANT(target, GIT_REF_LISTALL);

	target->Set(ref_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Reference::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, refArg);

	Reference *ref = static_cast<Reference*>(refArg->Value());
	ref->Wrap(args.This());
	ref->processInitData();

	return args.This();
}

Handle<Value> Reference::Rename(const Arguments& args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());

	REQ_ARGS(1);
	REQ_STR_ARG(0, newNameArg);

	CHECK_DELETED();

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		ref_request *request = new ref_request;

		request->ref = ref;
		request->callback = Persistent<Function>::New(callbackArg);
		request->name = new std::string(*newNameArg);

		ref->Ref();
		eio_custom(EIO_Rename, EIO_PRI_DEFAULT, EIO_AfterRename, request);
		ev_ref(EV_DEFAULT_UC);

		ref->unlock();

		return Undefined();
	}
	else {
		ref->repository_->lockRepository();
		int result = git_reference_rename(ref->ref_, *newNameArg);
		ref->repository_->unlockRepository();

		ref->unlock();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't rename ref.", result);
		}

		args.This()->ForceSet(name_symbol, String::New(*newNameArg),
				(PropertyAttribute)(ReadOnly | DontDelete));

		return scope.Close(Undefined());
	}
}

int Reference::EIO_Rename(eio_req *req) {
	ref_request *reqData = static_cast<ref_request*>(req->data);

	IF_ASYNC_CHECK_ISNT_DELETED()
		reqData->ref->repository_->lockRepository();
		reqData->error = git_reference_rename(reqData->ref->ref_, reqData->name->c_str());
		reqData->ref->repository_->unlockRepository();
		reqData->ref->unlock();
	}

	return 0;
}

int Reference::EIO_AfterRename(eio_req *req) {
	HandleScope scope;
	ref_request *reqData = static_cast<ref_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->ref->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't rename ref"), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->ref->handle_->ForceSet(name_symbol, String::New(reqData->name->c_str()),
				(PropertyAttribute)(ReadOnly | DontDelete));

 		callbackArgs[0] = Undefined();
 		callbackArgs[1] = True();
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData->name;
	delete reqData;

	return 0;
}

Handle<Value> Reference::Delete(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());

	CHECK_DELETED();

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		ref_request *request = new ref_request;

		request->ref = ref;
		request->callback = Persistent<Function>::New(callbackArg);

		ref->Ref();
		eio_custom(EIO_Delete, EIO_PRI_DEFAULT, EIO_AfterDelete, request);
		ev_ref(EV_DEFAULT_UC);

		ref->unlock();

		return Undefined();
	}
	else {
		ref->repository_->lockRepository();
		int result = git_reference_delete(ref->ref_);
		ref->repository_->unlockRepository();

		if(result != GIT_SUCCESS) {
			ref->unlock();
			THROW_GIT_ERROR("Couldn't delete ref.", result);
		}

		ref->repository_->referenceCache_->remove(ref->ref_);
		ref->deleted_ = true;

		ref->unlock();
	}

	return scope.Close(True());
}

int Reference::EIO_Delete(eio_req *req) {
	ref_request *reqData = static_cast<ref_request*>(req->data);

	IF_ASYNC_CHECK_ISNT_DELETED()
		reqData->ref->repository_->lockRepository();
		reqData->error = git_reference_delete(reqData->ref->ref_);
		reqData->ref->repository_->unlockRepository();

		if(reqData->error == GIT_SUCCESS) {
			reqData->ref->repository_->referenceCache_->remove(reqData->ref->ref_);
			reqData->ref->deleted_ = true;
		}

		reqData->ref->unlock();
	}

	return 0;
}

int Reference::EIO_AfterDelete(eio_req *req) {
	HandleScope scope;
	ref_request *reqData = static_cast<ref_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->ref->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't delete ref"), reqData->error);
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

Handle<Value> Reference::Resolve(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());

	CHECK_DELETED()

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length()-1, callbackArg);
		resolve_request *request = new resolve_request;

		request->ref = ref;
		request->callback = Persistent<Function>::New(callbackArg);

		ref->Ref();
		eio_custom(EIO_Resolve, EIO_PRI_DEFAULT, EIO_AfterResolve, request);
		ev_ref(EV_DEFAULT_UC);

		ref->unlock();

		return Undefined();
	}
	else {
		ref->repository_->lockRefs();

		git_reference *resolvedRef;
		ref->repository_->lockRepository();
		int result = git_reference_resolve(&resolvedRef, ref->ref_);
		ref->repository_->unlockRepository();

		ref->unlock();

		if(result != GIT_SUCCESS) {
			ref->repository_->unlockRefs();
			THROW_GIT_ERROR("Couldn't resolve ref.", result);
		}

		Handle<Value> resolvedRefObj = ref->repository_->referenceCache_->
				syncRequest(resolvedRef);
		ref->repository_->unlockRefs();
		return scope.Close(resolvedRefObj);
	}
}

int Reference::EIO_Resolve(eio_req *req) {
	resolve_request *reqData = static_cast<resolve_request*>(req->data);

	IF_ASYNC_CHECK_ISNT_DELETED()
		reqData->ref->repository_->lockRefs();
		reqData->ref->repository_->lockRepository();
		reqData->error = git_reference_resolve(&reqData->resolved, reqData->ref->ref_);
		reqData->ref->repository_->unlockRepository();
		reqData->ref->unlock();

		if(reqData->error == GIT_SUCCESS) {
			reqData->error = reqData->ref->repository_->referenceCache_->
					asyncRequest(reqData->resolved, &reqData->resolvedObj);
		}

		reqData->ref->repository_->unlockRefs();
	}

	return 0;
}

int Reference::EIO_AfterResolve(eio_req *req) {
	HandleScope scope;
	resolve_request *reqData = static_cast<resolve_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->ref->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		reqData->ref->repository_->unlockRefs();
 		Handle<Value> error = CreateGitError(String::New("Couldn't resolve ref"), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Undefined();
	}
	else {
		reqData->resolvedObj->ensureWrapped();
		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(reqData->resolvedObj->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> Reference::SetTarget(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());
	CHECK_DELETED();
	
	REQ_ARGS(1);
	
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length()-1, callbackArg);
		target_request *request = new target_request;

		request->ref = ref;
		request->callback = Persistent<Function>::New(callbackArg);
		request->target = new std::string(*String::Utf8Value(args[0]));

		ref->Ref();
		eio_custom(EIO_SetTarget, EIO_PRI_DEFAULT, EIO_AfterSetTarget, request);
		ev_ref(EV_DEFAULT_UC);

		ref->unlock();

		return Undefined();
	}
	else {
		int result = GIT_ERROR;

		// Macros ftw. I'm just gonna unlock the ref here, simply because
		// the req_*_arg macros throw errors and don't really care about unlocking
		// my ref.

		if(ref->type_ == GIT_REF_OID) {
			ref->unlock();
			REQ_OID_ARG(0, oidArg);
			CHECK_DELETED();
			result = git_reference_set_oid(ref->ref_, &oidArg);
		}
		else if(ref->type_ == GIT_REF_SYMBOLIC) {
			ref->unlock();
			REQ_STR_ARG(0, targetArg);
			CHECK_DELETED();
			result = git_reference_set_target(ref->ref_, *targetArg);
		}

		ref->unlock();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't set target.", result);
		}
		else {
			args.This()->ForceSet(target_symbol, args[0],
					(PropertyAttribute)(ReadOnly | DontDelete));
		}

		return scope.Close(True());
	}
}

int Reference::EIO_SetTarget(eio_req *req) {
	target_request *reqData = static_cast<target_request*>(req->data);

	IF_ASYNC_CHECK_ISNT_DELETED()
		reqData->ref->repository_->lockRepository();
		if(reqData->ref->type_ == GIT_REF_OID) {
			git_oid id;
			reqData->error = git_oid_mkstr(&id, reqData->target->c_str());

			if(reqData->error == GIT_SUCCESS) {
				reqData->error = git_reference_set_oid(reqData->ref->ref_, &id);
			}
		}
		else if(reqData->ref->type_ == GIT_REF_SYMBOLIC) {
			reqData->error = git_reference_set_target(reqData->ref->ref_,
					reqData->target->c_str());
		}
		reqData->ref->repository_->unlockRepository();

		reqData->ref->unlock();
	}

	return 0;
}

int Reference::EIO_AfterSetTarget(eio_req *req) {
	HandleScope scope;
	target_request *reqData = static_cast<target_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->ref->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't set ref target."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Undefined();
 		callbackArgs[1] = True();

 		reqData->ref->handle_->ForceSet(target_symbol, String::New(reqData->target->c_str()),
 				(PropertyAttribute)(ReadOnly | DontDelete));
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData->target;
	delete reqData;

	return 0;
}

void Reference::processInitData() {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	ref_data *refData = initData_;
	jsObject->Set(name_symbol, String::New(refData->name->c_str()),
			(PropertyAttribute)(ReadOnly | DontDelete));

	type_ = refData->type;
	jsObject->Set(type_symbol, Integer::New(refData->type),
			(PropertyAttribute)(ReadOnly | DontDelete));

	jsObject->Set(target_symbol, String::New(refData->target->c_str()),
			(PropertyAttribute)(ReadOnly | DontDelete));

	delete refData->name;
	delete refData->target;
	delete refData;
}

int Reference::doInit() {
	ref_data *data = initData_ = new ref_data;

	lock();
	repository_->lockRepository();

	data->name = new std::string(git_reference_name(ref_));
	data->type = git_reference_type(ref_);

	if(data->type == GIT_REF_OID) {
		char id[40];
		git_oid_fmt(id, git_reference_oid(ref_));
		data->target = new std::string(id, 40);
	}
	else if(data->type == GIT_REF_SYMBOLIC) {
		data->target = new std::string(git_reference_target(ref_));
	}

	repository_->unlockRepository();
	unlock();

	return GIT_SUCCESS;
}

void Reference::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
