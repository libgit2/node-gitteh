#include "odb_obj.h"
#include "odb.h"
#include <node_buffer.h>

namespace gitteh {

Persistent<FunctionTemplate> ODBObject::constructor_template;

static Persistent<String> odbobj_class_symbol;
static Persistent<String> id_symbol;
static Persistent<String> type_symbol;
static Persistent<String> data_symbol;

struct init_data {
	char id[40];
	std::string *type;
	int length;
	char *data;
};

struct save_request {
	Persistent<Function> callback;
	ObjectDatabase *odb;
	Persistent<Object> odbHandle;
	bool isNew;
	ODBObject *obj;
	void *data;
	int length;
	int error;
	char id[40];
	git_otype type;
};

void ODBObject::Init(Handle<Object> target) {
	HandleScope scope;

	odbobj_class_symbol = NODE_PSYMBOL("ODBObject");
	type_symbol = NODE_PSYMBOL("type");
	data_symbol = NODE_PSYMBOL("data");
	id_symbol = NODE_PSYMBOL("id");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(odbobj_class_symbol);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	target->Set(odbobj_class_symbol, constructor_template->GetFunction());
}

Handle<Value> ODBObject::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, objArg);

	ODBObject *obj = static_cast<ODBObject*>(objArg->Value());
	obj->Wrap(args.This());

	obj->processInitData();

	return args.This();
}

Handle<Value> ODBObject::SaveObject(Handle<Object> odbObject, ObjectDatabase *odb,
		Handle<Value> callback, bool isNew) {
	HandleScope scope;

	if(!odbObject->Get(data_symbol)->IsObject()) {
		THROW_ERROR("Data is required.");
	}

	if(!odbObject->Get(type_symbol)->IsString()) {
		THROW_ERROR("Type is required.");
	}

	git_otype type = git_object_string2type(*String::Utf8Value(odbObject->Get(type_symbol)));
	if(type == GIT_OBJ_BAD) {
		THROW_ERROR("Type is invalid.");
	}

	// Make sure the provided data is an instance of FastBuffer.
	Local<Object> globalObj = Context::GetCurrent()->Global();
		Local<Function> bufferConstructor = Local<Function>::Cast(
				globalObj->Get(String::New("Buffer")));

	Handle<Object> bufferObj = Handle<Object>::Cast(odbObject->Get(data_symbol));
	/*if(!bufferObj->GetPrototype()->Equals(bufferObj)) {
		THROW_ERROR("Data should be a Buffer");
	}*/

	int dataLen = bufferObj->GetIndexedPropertiesExternalArrayDataLength();
	void *data = bufferObj->GetIndexedPropertiesExternalArrayData();

	// TODO: checks on this data?

	if(callback->IsFunction()) {
		save_request *request = new save_request;
		request->callback = Persistent<Function>::New(Handle<Function>::Cast(callback));
		request->odb = odb;
		request->odbHandle = Persistent<Object>::New(odb->handle_);
		request->isNew = isNew;
		request->type = type;
		if(!isNew) {
			request->obj = ObjectWrap::Unwrap<ODBObject>(odbObject);
			request->obj->Ref();
		}
		else {
			request->obj = NULL;
		}

		request->data = data;
		request->length = dataLen;

		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_oid newId;

		odb->lockOdb();
		int result = git_odb_write(&newId, odb->odb_, data, dataLen, type);
		odb->unlockOdb();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't save object.", result);
		}

		char newIdStr[40];
		git_oid_fmt(newIdStr, &newId);
		if(isNew) {
			Handle<Function> getObjFn = Handle<Function>::Cast(
					odb->handle_->Get(String::New("get")));
			Handle<Value> arg = String::New(newIdStr, 40);
			return scope.Close(getObjFn->Call(odb->handle_, 1, &arg));
		}
		else {
			odbObject->ForceSet(id_symbol, String::New(newIdStr, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));

			return scope.Close(True());
		}
	}
}


Handle<Value> ODBObject::Save(const Arguments &args) {
	HandleScope scope;
	ODBObject *obj = ObjectWrap::Unwrap<ODBObject>(args.This());

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(SaveObject(args.This(), obj->odb_, callback, false));
}

int ODBObject::EIO_Save(eio_req *req) {
	save_request *reqData = static_cast<save_request*>(req->data);

	git_oid newId;
	reqData->odb->lockOdb();
	reqData->error = git_odb_write(&newId, reqData->odb->odb_,
			reqData->data, reqData->length, reqData->type);
	reqData->odb->unlockOdb();

	if(reqData->error == GIT_SUCCESS) {
		git_oid_fmt(reqData->id, &newId);
	}

	return 0;
}

int ODBObject::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_request *reqData = static_cast<save_request*>(req->data);

	reqData->odbHandle.Dispose();
	ev_unref(EV_DEFAULT_UC);
 	if(reqData->obj != NULL) reqData->obj->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't save object."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Null();

		if(reqData->isNew) {
			Handle<Function> getObjectFn = Handle<Function>::Cast(
					reqData->odb->handle_->Get(String::New("get")));
			Handle<Value> arg = String::New(reqData->id, 40);
	 		callbackArgs[1] = getObjectFn->Call(reqData->odb->handle_, 1, &arg);
		}
		else {
			reqData->obj->handle_->ForceSet(id_symbol, String::New(reqData->id, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));
			callbackArgs[1] = True();
		}
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

ODBObject::ODBObject(git_odb_object *obj) {
	obj_ = obj;
}

ODBObject::~ODBObject() {
	odb_->lockOdb();
	git_odb_object_close(obj_);
	odb_->unlockOdb();
}

void ODBObject::setOwner(ObjectDatabase *odb) {
	odb_ = odb;
}

int ODBObject::doInit() {
	init_data *data = initData_ = new init_data;

	const git_oid *oid = git_odb_object_id(obj_);
	git_oid_fmt(data->id, oid);

	git_otype type = git_odb_object_type(obj_);
	data->type = new std::string(git_object_type2string(type));

	data->length = git_odb_object_size(obj_);
	data->data = new char[data->length];
	memcpy(data->data, git_odb_object_data(obj_), data->length);

	return GIT_SUCCESS;
}

static void BufferDataFreeCallback(char *data, void *hint) {
	delete [] data;
}

void ODBObject::processInitData() {
	HandleScope scope;

	Handle<Object> jsObject = handle_;

	Buffer *buf = Buffer::New(static_cast<char*>(initData_->data),
			initData_->length, BufferDataFreeCallback, NULL);
	Local<Object> globalObj = Context::GetCurrent()->Global();
	Local<Function> bufferConstructor = Local<Function>::Cast(
			globalObj->Get(String::New("Buffer")));
	Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(
			initData_->length), Integer::New(0) };
	Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
	jsObject->Set(data_symbol, actualBuffer);

	jsObject->Set(type_symbol, String::New(initData_->type->c_str()));

	jsObject->Set(id_symbol, String::New(initData_->id, 40),
			(PropertyAttribute)(DontDelete | ReadOnly));

	delete initData_->type;
	delete initData_;

}

} // namespace gitteh
