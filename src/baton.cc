#include "baton.h"
#include "gitteh.h"
#include <iostream>

namespace gitteh {

Baton::Baton() {
	errorCode = 0;
	req.data = this;
}

Baton::~Baton() {
	if(!callback.IsEmpty()) {
		callback.Dispose();
		callback.Clear();
	}
}

void Baton::setCallback(Handle<Value> val) {
	NanScope();

	callback = Persistent<Function>::New(Handle<Function>::Cast(val));
}

bool Baton::isErrored() {
	return errorCode != 0;
}

void Baton::setError(const git_error *err) {
	errorCode = err->klass;
	errorString = string(err->message);
}

Handle<Object> Baton::createV8Error() {
	NanEscapableScope();

	assert(errorCode != 0);
	Handle<Object> errObj = Handle<Object>::Cast(NanThrowError(
		NanNew<String>(errorString.c_str())));
	errObj->Set(NanNew<String>("code"), NanNew<Number>(errorCode));
	return NanEscapeScope(errObj);
}

void Baton::defaultCallback() {
	NanScope();

	if(isErrored()) {
		Handle<Value> argv[] = { createV8Error() };
		FireCallback(callback, 1, argv);
	}
	else {
		Handle<Value> argv[] = { Undefined() };
		FireCallback(callback, 1, argv);
	}
}

}; // namespace gitteh
