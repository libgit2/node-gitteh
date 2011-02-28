#include "rawobj.h"

Persistent<FunctionTemplate> RawObject::constructor_template;

void RawObject::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("RawObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->PrototypeTemplate()->SetAccessor(RAWOBJ_TYPE_SYMBOL, TypeGetter);
	t->PrototypeTemplate()->SetAccessor(RAWOBJ_DATA_SYMBOL, DataGetter);
}

Handle<Value> RawObject::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theObj);

	RawObject *obj = new RawObject();
	obj->obj_ = *((git_rawobj*)theObj->Value());

	obj->Wrap(args.This());
	obj->MakeWeak();

	return args.This();
}

Handle<Value> RawObject::TypeGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	RawObject *rawObject = ObjectWrap::Unwrap<RawObject>(info.This());

	Local<Number> type = Integer::New(rawObject->obj_.type);
	return scope.Close(type);
}

Handle<Value> RawObject::DataGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	RawObject *rawObject = ObjectWrap::Unwrap<RawObject>(info.This());
	Buffer *buf = Buffer::New(rawObject->obj_.len);
	memcpy(Buffer::Data(buf), rawObject->obj_.data, rawObject->obj_.len);

	return scope.Close(buf->handle_);
}
