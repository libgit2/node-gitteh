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
	time_t sigTime = sigDate->NumberValue();

	String::Utf8Value sigName(sigObj->Get(SIG_NAME_PROPERTY));
	if(!sigName.length())
		return NULL;

	String::Utf8Value sigEmail(sigObj->Get(SIG_EMAIL_PROPERTY));
	if(!sigEmail.length())
		return NULL;

	git_signature *sig = git_signature_new(*sigName,
			*sigEmail, sigTime, 0);

	return NULL;
}

} // namespace gitteh

#endif // GITTEH_SIGNATURE_H
