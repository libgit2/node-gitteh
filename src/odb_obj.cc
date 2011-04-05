#include "odb_obj.h"
#include "odb.h"
#include <node_buffer.h>

namespace gitteh {

Persistent<FunctionTemplate> ODBObject::constructor_template;

static Persistent<String> odbobj_class_symbol;
static Persistent<String> type_symbol;
static Persistent<String> data_symbol;

struct init_data {
	char id[40];
	std::string *type;
	int length;
	char *data;
};

void ODBObject::Init(Handle<Object> target) {
	HandleScope scope;

	odbobj_class_symbol = NODE_PSYMBOL("ODBObject");
	type_symbol = NODE_PSYMBOL("type");
	data_symbol = NODE_PSYMBOL("data");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(odbobj_class_symbol);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

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

	delete initData_->type;
	delete initData_;

}

} // namespace gitteh
