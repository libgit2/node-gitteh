#include "rawobj.h"
#include "repository.h"
#include <node_buffer.h>

#define ID_PROPERTY String::NewSymbol("id")
#define DATA_PROPERTY String::NewSymbol("data")
#define TYPE_PROPERTY String::NewSymbol("type")

namespace gitteh {

Persistent<FunctionTemplate> RawObject::constructor_template;

void RawObject::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("RawObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> RawObject::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theObj);

	RawObject *obj = new RawObject();
	obj->Wrap(args.This());

	obj->obj_ = static_cast<git_rawobj*>(theObj->Value());

	if(obj->obj_->type != GIT_OBJ_BAD) {
		git_oid objId;
		int res = git_rawobj_hash(&objId, obj->obj_);
		if(res != GIT_SUCCESS)
			THROW_GIT_ERROR("Couldn't get rawobj id.", res);

		const char* oidStr = git_oid_allocfmt(&objId);
		args.This()->Set(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

		Buffer *buf = Buffer::New(static_cast<char*>(obj->obj_->data), obj->obj_->len);

		Local<Object> globalObj = Context::GetCurrent()->Global();
		Local<Function> bufferConstructor = Local<Function>::Cast(globalObj->Get(String::New("Buffer")));
		Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(obj->obj_->len), Integer::New(0) };
		Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
		args.This()->Set(DATA_PROPERTY, actualBuffer);
	}
	else {
		args.This()->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		args.This()->Set(DATA_PROPERTY, Null());
	}

	args.This()->Set(TYPE_PROPERTY, String::New(git_object_type2string(obj->obj_->type)));

	return args.This();
}

Handle<Value> RawObject::Save(const Arguments& args) {
	HandleScope scope;

	RawObject *obj = ObjectWrap::Unwrap<RawObject>(args.This());

	Handle<Object> thisObj = args.This();

	CHECK_PROPERTY(DATA_PROPERTY);
	CHECK_PROPERTY(TYPE_PROPERTY);

	if(!Buffer::HasInstance(thisObj->Get(DATA_PROPERTY))) {
		THROW_ERROR("Data is not a valid Buffer.");
	}

	Handle<Object> bufferObj = Handle<Object>::Cast(
			thisObj->Get(DATA_PROPERTY));

	git_otype type = git_object_string2type(*String::Utf8Value(
			thisObj->Get(TYPE_PROPERTY)));

	if(type == GIT_OBJ_BAD) {
		THROW_ERROR("Invalid type.");
	}

	obj->obj_->type = type;
	obj->obj_->len = Buffer::Length(bufferObj);
	obj->obj_->data = Buffer::Data(bufferObj);

	git_oid newId;
	int res = git_odb_write(&newId, obj->repository_->odb_, obj->obj_);
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't save raw object.", res);

	const char* oidStr = git_oid_allocfmt(&newId);
	args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));
}

RawObject::~RawObject() {
	delete obj_;
}

} // namespace gitteh
