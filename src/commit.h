#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "git_object.h"

namespace gitteh {
	namespace Commit {
		void Init(Handle<Object>);
		Handle<Object> Create(git_commit*);
	};
}; // namespace gitteh

#endif // GITTEH_COMMIT_H
