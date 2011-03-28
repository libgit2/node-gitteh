#include "blob.h"
#include "repository.h"
#include <node_buffer.h>

namespace gitteh {

Persistent<FunctionTemplate> Blob::constructor_template;

static Persistent<String> id_symbol;
static Persistent<String> data_symbol;

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

	if(!blobObject->Has(data_symbol)) {
		THROW_ERROR("Data is required.");
	}


}

Handle<Value> Blob::Save(const Arguments &args) {

}

int Blob::EIO_Save(eio_req *req) {

}

int Blob::EIO_AfterSave(eio_req *req) {

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
