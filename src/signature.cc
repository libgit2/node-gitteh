#include "signature.h"

static Persistent<String> sig_name_symbol;
static Persistent<String> sig_email_symbol;
static Persistent<String> sig_time_symbol;
static Persistent<String> sig_offset_symbol;

namespace gitteh {
void SignatureInit() {
		sig_name_symbol = NODE_PSYMBOL("name");
		sig_email_symbol = NODE_PSYMBOL("email");
		sig_time_symbol = NODE_PSYMBOL("time");
		sig_offset_symbol = NODE_PSYMBOL("offset");
	}

	Handle<Object> CreateSignature(const git_signature *sig) {
		Handle<Object> sigObj = Object::New();
		ImmutableSet(sigObj, sig_name_symbol, CastToJS(sig->name));
		ImmutableSet(sigObj, sig_email_symbol, CastToJS(sig->email));
		ImmutableSet(sigObj, sig_time_symbol, Date::New(sig->when.time * 1000));
		ImmutableSet(sigObj, sig_offset_symbol, CastToJS(sig->when.offset));
		return sigObj;
	}
};
