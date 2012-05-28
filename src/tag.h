#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"

namespace gitteh {
	namespace Tag {
		void Init(Handle<Object>);
		Handle<Value> Create(git_tag*);
	}
};

#endif // GITTEH_TAG_H
