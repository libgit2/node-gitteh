#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "nan.h"
#include "gitteh.h"

namespace gitteh {
	namespace Tree {
		void Init(Handle<Object>);
		Handle<Object> Create(git_tree*);
	};
};

#endif // GITTEH_TREE_H
