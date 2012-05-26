#include "baton.h"
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
	HandleScope scope;

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
	HandleScope scope;

	assert(errorCode != 0);
	Handle<Object> errObj = Handle<Object>::Cast(Exception::Error(
		String::New(errorString.c_str())));
	errObj->Set(String::New("code"), Integer::New(errorCode));
	return scope.Close(errObj);
}

}; // namespace gitteh
