#include "odb.h"
#include "rawobj.h"

Persistent<FunctionTemplate> ObjectDatabase::constructor_template;

void ObjectDatabase::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("ObjectDatabase"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "read", Read);
}

Handle<Value> ObjectDatabase::New(const Arguments& args) {
	HandleScope scope;

	REQ_EXT_ARG(0, theOdb);

	ObjectDatabase *odb = new ObjectDatabase();
	odb->odb_ = (git_odb*)theOdb->Value();

	odb->Wrap(args.This());
	odb->MakeWeak();

	return args.This();
}

Handle<Value> ObjectDatabase::Read(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_OID_ARG(0, oid);

	ObjectDatabase *odb = ObjectWrap::Unwrap<ObjectDatabase>(args.This());

	git_rawobj obj;
	if(git_odb_read(&obj, odb->odb_, &oid) == GIT_ENOTFOUND) {
		return Null();
	}

	Local<Value> arg = External::New(&obj);
	Persistent<Object> result(RawObject::constructor_template->GetFunction()->NewInstance(1, &arg));
	return scope.Close(result);
}

ObjectDatabase::ObjectDatabase() {
}
