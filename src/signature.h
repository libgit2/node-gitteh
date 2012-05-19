#ifndef GITTEH_SIGNATURE_H
#define GITTEH_SIGNATURE_H

#include "gitteh.h"

namespace gitteh {

static inline git_signature *GetSignatureFromProperty(Handle<Object> object,
		Handle<String> propertyName) {
	Handle<Value> property = object->Get(propertyName);
	if(!property->IsObject())
		return NULL;
	Handle<Object> sigObj = Handle<Object>::Cast(property);

	Handle<Date> sigDate = Handle<Date>::Cast(sigObj->Get(SIG_TIME_PROPERTY));
	if(!sigDate->IsDate())
		return NULL;
	time_t sigTime = NODE_V8_UNIXTIME(sigDate);

	int offset = 0;
	if(sigObj->Has(SIG_OFFSET_PROPERTY)) {
		offset = sigObj->Get(SIG_OFFSET_PROPERTY)->IntegerValue();
	}

	String::Utf8Value sigName(sigObj->Get(SIG_NAME_PROPERTY));
	if(!sigName.length())
		return NULL;

	String::Utf8Value sigEmail(sigObj->Get(SIG_EMAIL_PROPERTY));
	if(!sigEmail.length())
		return NULL;

	// TODO: Error handling.
	git_signature *sig;
	int error = git_signature_new(&sig, *sigName, *sigEmail, sigTime, offset);


	return sig;
}

} // namespace gitteh

#endif // GITTEH_SIGNATURE_H
