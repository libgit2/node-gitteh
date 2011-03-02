#include "rawobj.h"

Persistent<FunctionTemplate> RawObject::constructor_template;

void RawObject::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("RawObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> RawObject::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theObj);

	RawObject *obj = new RawObject();
	obj->obj_ = static_cast<git_rawobj*>(theObj->Value());

	Buffer *buf = Buffer::New(obj->obj_->len);
	memcpy(Buffer::Data(buf), obj->obj_->data, obj->obj_->len);

	Local<Object> globalObj = Context::GetCurrent()->Global();
	Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
	Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(obj->obj_->len), Integer::New(0) };
	Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);

	args.This()->Set(String::New("data"), actualBuffer);

	args.This()->Set(String::New("type"), String::New(git_object_type2string(obj->obj_->type)), ReadOnly);

	obj->Wrap(args.This());
	return args.This();
}
