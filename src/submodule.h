#ifndef GITTEH_SUBMODULE_H
#define GITTEH_SUBMODULE_H

#include "gitteh.h"

namespace gitteh {
	namespace Submodule {
		void Init(Handle<Object>);
		Handle<Object> Create(git_submodule*);
	}
};

#endif // GITTEH_SUBMODULE_H
