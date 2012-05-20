#include "baton.h"

namespace gitteh {

Baton::Baton() {
	error.klass = 0;
	req.data = this;
}

Baton::~Baton() {
	if(!callback.IsEmpty()) {
		callback.Dispose();
		callback.Clear();
	}
}

void Baton::setCallback(Handle<Value> val) {
	callback = Persistent<Function>::New(Handle<Function>::Cast(val));
}

bool Baton::isErrored() {
	return error.klass != 0;
}

Handle<Object> Baton::createV8Error() {
	assert(error.klass != 0);
	Handle<Object> errObj = Handle<Object>::Cast(Exception::Error(
		String::New(error.message)));
	errObj->Set(String::New("code"), Integer::New(error.klass));
	return errObj;
}

}; // namespace gitteh
