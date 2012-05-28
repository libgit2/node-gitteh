#ifndef GITTEH_SIGNATURE_H
#define GITTEH_SIGNATURE_H

#include "gitteh.h"

namespace gitteh {
	namespace Signature {
		void Init();
		Handle<Object> Create(const git_signature *sig);
	};
};

namespace cvv8 {
	template<>
	struct NativeToJS<git_signature> {
		Handle<Value> operator() (git_signature const *sig) const;
	};
};

#endif // GITTEH_SIGNATURE_H
