#ifndef GITTEH_SIGNATURE_H
#define GITTEH_SIGNATURE_H

#include "gitteh.h"

namespace gitteh {
	namespace Signature {
		void Init();
		Handle<Object> Create(const git_signature *sig);
	};
};

#endif // GITTEH_SIGNATURE_H
