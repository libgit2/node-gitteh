#include "signature.h"

namespace gitteh {
	namespace Signature {
		static Persistent<String> name_symbol;
		static Persistent<String> email_symbol;
		static Persistent<String> time_symbol;
		static Persistent<String> offset_symbol;

		void Init() {
			name_symbol 	= NODE_PSYMBOL("name");
			email_symbol 	= NODE_PSYMBOL("email");
			time_symbol 	= NODE_PSYMBOL("time");
			offset_symbol 	= NODE_PSYMBOL("offset");
		}

		Handle<Object> Create(const git_signature *sig) {
			HandleScope scope;
			Handle<Object> o = Object::New();
			ImmutableSet(o, name_symbol, CastToJS(sig->name));
			ImmutableSet(o, email_symbol, CastToJS(sig->email));
			ImmutableSet(o, time_symbol, Date::New(sig->when.time * 1000));
			ImmutableSet(o, offset_symbol, CastToJS(sig->when.offset));
			return scope.Close(o);
		}
	};
};
