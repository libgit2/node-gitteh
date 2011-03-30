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

#include "index.h"
#include "index_entry.h"
#include "repository.h"

#define LENGTH_PROPERTY String::NewSymbol("entryCount")

namespace gitteh {

struct index_data {
	int entryCount;
};

struct index_request {
	Persistent<Function> callback;
	Index *indexObj;
	int error;
};

struct entry_request {
	Persistent<Function> callback;
	Index *indexObj;
	int index;
	std::string *name;
	git_index_entry *entry;
	int error;
};

struct add_entry_request {
	Persistent<Function> callback;
	Index *indexObj;
	std::string *path;
	int stage;
	int error;
};

struct insert_entry_request {
	Persistent<Function> callback;
	Index *indexObj;
	git_index_entry entry;
	int error;
};

Persistent<FunctionTemplate> Index::constructor_template;

Index::Index() {
	entryFactory_ = new ObjectFactory<Index, IndexEntry, git_index_entry>(this);
}

Index::~Index() {
	repository_->notifyIndexDead();
	git_index_free(index_);
}

void Index::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Index"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getEntry", GetEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "findEntry", FindEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "addEntry", AddEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "write", Write);

	NODE_DEFINE_CONSTANT(target, GIT_IDXENTRY_NAMEMASK);
	NODE_DEFINE_CONSTANT(target, GIT_IDXENTRY_STAGEMASK);
	NODE_DEFINE_CONSTANT(target, GIT_IDXENTRY_EXTENDED);
	NODE_DEFINE_CONSTANT(target, GIT_IDXENTRY_VALID);
	NODE_DEFINE_CONSTANT(target, GIT_IDXENTRY_STAGESHIFT);
}

Handle<Value> Index::New(const Arguments& args) {
	HandleScope scope;

	Index *index = new Index();
	index->Wrap(args.This());

	return args.This();
}

Handle<Value> Index::GetEntry(const Arguments& args) {
	HandleScope scope;
	Index *index = ObjectWrap::Unwrap<Index>(args.This());

	REQ_ARGS(1)
	REQ_INT_ARG(0, indexArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		entry_request *request = new entry_request;

		request->indexObj = index;
		request->callback = Persistent<Function>::New(callbackArg);
		request->index = indexArg;
		request->entry = NULL;

		index->Ref();
		eio_custom(EIO_GetEntry, EIO_PRI_DEFAULT, EIO_AfterGetEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		index->repository_->lockRepository();
		git_index_entry *entry = git_index_get(index->index_, indexArg);
		index->repository_->unlockRepository();

		if(entry == NULL) {
			THROW_ERROR("Invalid entry.");
		}

		IndexEntry *entryObject = index->entryFactory_->syncRequestObject(entry);
		return scope.Close(entryObject->handle_);
	}
}

int Index::EIO_GetEntry(eio_req *req) {
	entry_request *reqData = static_cast<entry_request*>(req->data);

	reqData->indexObj->repository_->lockRepository();
	reqData->entry = git_index_get(reqData->indexObj->index_, reqData->index);
	reqData->indexObj->repository_->unlockRepository();

	return 0;
}

int Index::EIO_AfterGetEntry(eio_req *req) {
	HandleScope scope;
	entry_request *reqData = static_cast<entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->indexObj->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->entry == NULL) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't get index entry."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();

 		TRIGGER_CALLBACK();

 		reqData->callback.Dispose();
	}
	else {
		reqData->indexObj->entryFactory_->asyncRequestObject(
				reqData->entry, reqData->callback);
	}

	delete reqData;

	return 0;
}

Handle<Value> Index::FindEntry(const Arguments& args) {
	HandleScope scope;
	Index *indexObj = ObjectWrap::Unwrap<Index>(args.This());

	REQ_ARGS(1)
	REQ_STR_ARG(0, nameArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		entry_request *request = new entry_request;

		request->indexObj = indexObj;
		request->callback = Persistent<Function>::New(callbackArg);
		request->name = new std::string(*nameArg);
		request->entry = NULL;

		indexObj->Ref();
		eio_custom(EIO_FindEntry, EIO_PRI_DEFAULT, EIO_AfterFindEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		indexObj->repository_->lockRepository();
		int index = git_index_find(indexObj->index_, *nameArg);

		if(index == GIT_ENOTFOUND) {
			indexObj->repository_->unlockRepository();
			THROW_ERROR("Invalid path.");
		}

		git_index_entry *entry = git_index_get(indexObj->index_, index);
		indexObj->repository_->unlockRepository();

		if(entry == NULL) {
			THROW_ERROR("Unknown error.");
		}

		IndexEntry *entryObject = indexObj->entryFactory_->syncRequestObject(entry);
		return scope.Close(entryObject->handle_);
	}
}

int Index::EIO_FindEntry(eio_req *req) {
	entry_request *reqData = static_cast<entry_request*>(req->data);

	reqData->indexObj->repository_->lockRepository();
	int index = git_index_find(reqData->indexObj->index_, reqData->name->c_str());

	if(index != GIT_ENOTFOUND) {
		reqData->entry = git_index_get(reqData->indexObj->index_, index);
	}
	else {
		reqData->entry = NULL;
	}

	delete reqData->name;
	reqData->indexObj->repository_->unlockRepository();

	return 0;
}

int Index::EIO_AfterFindEntry(eio_req *req) {
	HandleScope scope;
	entry_request *reqData = static_cast<entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
	reqData->indexObj->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->entry == NULL) {
		Handle<Value> error = Exception::Error(String::New("Couldn't find index entry."));
		callbackArgs[0] = error;
		callbackArgs[1] = Null();

		TRIGGER_CALLBACK();

		reqData->callback.Dispose();
	}
	else {
		reqData->indexObj->entryFactory_->asyncRequestObject(
				reqData->entry, reqData->callback);
	}

	delete reqData;

	return 0;
}

Handle<Value> Index::AddEntry(const Arguments &args) {
	HandleScope scope;
	Index *index = ObjectWrap::Unwrap<Index>(args.This());

	REQ_ARGS(2);
	REQ_STR_ARG(0, pathArg);
	REQ_INT_ARG(1, stageArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		add_entry_request *request = new add_entry_request;

		request->indexObj = index;
		request->callback = Persistent<Function>::New(callbackArg);
		request->path = new std::string(*pathArg);
		request->stage = stageArg;

		index->Ref();
		eio_custom(EIO_AddEntry, EIO_PRI_DEFAULT, EIO_AfterAddEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		index->repository_->lockRepository();
		int result = git_index_add(index->index_, *pathArg, stageArg);
		index->repository_->unlockRepository();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't add index entry.", result);
		}

 	 	index->updateEntryCount();

		return True();
	}
}

int Index::EIO_AddEntry(eio_req *req) {
	add_entry_request *reqData = static_cast<add_entry_request*>(req->data);

	reqData->indexObj->repository_->lockRepository();
	reqData->error = git_index_add(reqData->indexObj->index_, reqData->path->c_str(),
			reqData->stage);
	reqData->indexObj->repository_->unlockRepository();

	delete reqData->path;

	return 0;
}

int Index::EIO_AfterAddEntry(eio_req *req) {
	HandleScope scope;
	add_entry_request *reqData = static_cast<add_entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->indexObj->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't add entry."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Undefined();
 		callbackArgs[1] = True();

 	 	reqData->indexObj->updateEntryCount();
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> Index::Write(const Arguments& args) {
	HandleScope scope;
	Index *index = ObjectWrap::Unwrap<Index>(args.This());

	if(HAS_CALLBACK_ARG) {
		index_request *request = new index_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->indexObj = index;
		request->callback = Persistent<Function>::New(callbackArg);

		index->Ref();
		eio_custom(EIO_Write, EIO_PRI_DEFAULT, EIO_AfterWrite, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		index->repository_->lockRepository();
		int result = git_index_write(index->index_);
		index->repository_->unlockRepository();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't write index.", result);
		}

		return True();
	}
}

int Index::EIO_Write(eio_req *req) {
	index_request *reqData = static_cast<index_request*>(req->data);

	reqData->indexObj->repository_->lockRepository();
	reqData->error = git_index_write(reqData->indexObj->index_);
	reqData->indexObj->repository_->unlockRepository();

	return 0;
}

int Index::EIO_AfterWrite(eio_req *req) {
	HandleScope scope;
	index_request *reqData = static_cast<index_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->indexObj->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't write index."), reqData->error);
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

void Index::updateEntryCount() {
	HandleScope scope;

	repository_->lockRepository();
	entryCount_ = git_index_entrycount(index_);
	repository_->unlockRepository();

	handle_->ForceSet(LENGTH_PROPERTY, Integer::New(entryCount_),
			(PropertyAttribute)(ReadOnly | DontDelete));
}

void Index::processInitData(void *data) {
	if(data != NULL) {
		index_data *indexData = static_cast<index_data*>(data);

		entryCount_ = indexData->entryCount;
		handle_->Set(LENGTH_PROPERTY, Integer::New(indexData->entryCount),
				(PropertyAttribute)(ReadOnly | DontDelete));

		delete indexData;
	}
}

void *Index::loadInitData() {
	repository_->lockRepository();
	initError_ = git_repository_index(&index_, repository_->repo_);
	repository_->unlockRepository();
	if(initError_ == GIT_EBAREINDEX) {
		repository_->lockRepository();
		initError_ = git_index_open_bare(&index_, repository_->path_);
		repository_->unlockRepository();
	}

	if(initError_ != GIT_SUCCESS) {
		return NULL;
	}

	index_data *data = new index_data;

	repository_->lockRepository();
	git_index_read(index_);
	data->entryCount = git_index_entrycount(index_);
	repository_->unlockRepository();

	return data;
}

void Index::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
