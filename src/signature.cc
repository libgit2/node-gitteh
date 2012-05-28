#include "signature.h"

static Persistent<String> name_symbol;
static Persistent<String> email_symbol;
static Persistent<String> time_symbol;
static Persistent<String> offset_symbol;

namespace gitteh {
	namespace Signature {
		void Init() {
			name_symbol 	= NODE_PSYMBOL("name");
			email_symbol 	= NODE_PSYMBOL("email");
			time_symbol 	= NODE_PSYMBOL("time");
			offset_symbol 	= NODE_PSYMBOL("offset");
		}
	};
};

namespace cvv8 {
	Handle<Value> NativeToJS<git_signature>::operator() (git_signature const *sig) const {
		HandleScope scope;
		Handle<Object> o = Object::New();
		o->Set(name_symbol, CastToJS(sig->name));
		o->Set(email_symbol, CastToJS(sig->email));
		o->Set(time_symbol, Date::New(sig->when.time * 1000));
		o->Set(offset_symbol, CastToJS(sig->when.offset));
		return scope.Close(o);
	}
};
