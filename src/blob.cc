#include "blob.h"
#include "repository.h"
#include <node_buffer.h>

namespace gitteh {

Persistent<FunctionTemplate> Blob::constructor_template;

static Persistent<String> id_symbol;
static Persistent<String> data_symbol;

struct save_blob_request {
	Persistent<Function> callback;
	Repository *repo;
	Persistent<Object> repoHandle;
	Blob *blob;
	void *data;
	int length;
	char id[40];
	bool isNew;
	int error;
};

Blob::Blob() : GitObjectWrap() {

}

Blob::~Blob() {
	repository_->lockRepository();
	git_object_close((git_object*)blob_);
	repository_->unlockRepository();
}

void Blob::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	t->SetClassName(String::NewSymbol("Blob"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	id_symbol = NODE_PSYMBOL("id");
	data_symbol = NODE_PSYMBOL("data");
}

Handle<Value> Blob::New(const Arguments &args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, blobArg);

	Blob *blob = new Blob();
	blob->Wrap(args.This());
	blob->blob_ = static_cast<git_blob*>(blobArg->Value());

	return args.This();
}

Handle<Value> Blob::SaveObject(Handle<Object> blobObject, Repository *repo,
		Handle<Value> callback, bool isNew) {
	HandleScope scope;

	if(!blobObject->Get(data_symbol)->IsObject()) {
		THROW_ERROR("Data is required.");
	}

	// Make sure the provided data is an instance of FastBuffer.
	Local<Object> globalObj = Context::GetCurrent()->Global();
		Local<Function> bufferConstructor = Local<Function>::Cast(
				globalObj->Get(String::New("Buffer")));

	Handle<Object> bufferObj = Handle<Object>::Cast(blobObject->Get(data_symbol));
	/*if(!bufferObj->GetPrototype()->Equals(bufferObj)) {
		THROW_ERROR("Data should be a Buffer");
	}*/

	int dataLen = bufferObj->GetIndexedPropertiesExternalArrayDataLength();
	void *data = bufferObj->GetIndexedPropertiesExternalArrayData();

	// TODO: checks on this data?

	if(callback->IsFunction()) {
		save_blob_request *request = new save_blob_request;
		request->callback = Persistent<Function>::New(Handle<Function>::Cast(callback));
		request->repo = repo;
		request->repoHandle = Persistent<Object>::New(repo->handle_);
		request->isNew = isNew;
		if(!isNew) {
			request->blob = ObjectWrap::Unwrap<Blob>(blobObject);
			request->blob->Ref();
		}
		else {
			request->blob = NULL;
		}

		request->data = data;
		request->length = dataLen;

		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_oid newId;

		repo->lockRepository();
		int result = git_blob_create_frombuffer(&newId, repo->repo_, data, dataLen);
		repo->unlockRepository();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't save blob.", result);
		}

		char newIdStr[40];
		git_oid_fmt(newIdStr, &newId);
		if(isNew) {
			Handle<Function> getBlobFn = Handle<Function>::Cast(
					repo->handle_->Get(String::New("getBlob")));
			Handle<Value> arg = String::New(newIdStr, 40);
			return scope.Close(getBlobFn->Call(repo->handle_, 1,
					&arg));
		}
		else {
			blobObject->ForceSet(id_symbol, String::New(newIdStr, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));

			return scope.Close(True());
		}
	}
}

Handle<Value> Blob::Save(const Arguments &args) {
	HandleScope scope;
	Blob *blob = ObjectWrap::Unwrap<Blob>(args.This());

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(SaveObject(args.This(), blob->repository_, callback, false));
}

int Blob::EIO_Save(eio_req *req) {
	save_blob_request *reqData = static_cast<save_blob_request*>(req->data);

	git_oid newId;
	reqData->repo->lockRepository();
	reqData->error = git_blob_create_frombuffer(&newId, reqData->repo->repo_,
			reqData->data, reqData->length);
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		git_oid_fmt(reqData->id, &newId);
	}

	return 0;
}

int Blob::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_blob_request *reqData = static_cast<save_blob_request*>(req->data);

	reqData->repoHandle.Dispose();
	ev_unref(EV_DEFAULT_UC);
 	if(reqData->blob != NULL) reqData->blob->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't save blob."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Null();

		if(reqData->isNew) {
			Handle<Function> getBlobFn = Handle<Function>::Cast(
					reqData->repo->handle_->Get(String::New("getBlob")));
			Handle<Value> arg = String::New(reqData->id, 40);
	 		callbackArgs[1] = getBlobFn->Call(reqData->repo->handle_, 1, &arg);
		}
		else {
			reqData->blob->handle_->ForceSet(id_symbol, String::New(reqData->id, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));
			callbackArgs[1] = True();
		}
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

struct blob_data {
	char id[40];
	int length;
	void *data;
};

void Blob::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	blob_data *blobData = static_cast<blob_data*>(data);

	jsObject->Set(id_symbol, String::New(blobData->id, 40),
			(PropertyAttribute)(ReadOnly | DontDelete));

	Buffer *buf = Buffer::New(static_cast<char*>(blobData->data),
			blobData->length);
	Local<Object> globalObj = Context::GetCurrent()->Global();
	Local<Function> bufferConstructor = Local<Function>::Cast(
			globalObj->Get(String::New("Buffer")));
	Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(
			blobData->length), Integer::New(0) };
	Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
	jsObject->Set(data_symbol, actualBuffer);

	delete blobData;
}

void* Blob::loadInitData() {
	blob_data *data = new blob_data;

	repository_->lockRepository();
	const git_oid *id = git_object_id((git_object*)blob_);
	git_oid_fmt(data->id, id);

	data->length = git_blob_rawsize(blob_);
	data->data = const_cast<void*>(git_blob_rawcontent(blob_));

	repository_->unlockRepository();

	return data;
}

void Blob::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
