#ifndef GITTEH_GIT_OBJECT_H
#define GITTEH_GIT_OBJECT_H

#include "gitteh.h"
#include "repository.h"
#include <node_object_wrap.h>
#include <git2.h>

namespace gitteh {
	class GitObjectCache;

	class GitObject : public ObjectWrap {
	public:
		friend class GitObjectCache;
		git_otype type_;
		git_oid oid_;

		GitObject(git_object *obj);
		~GitObject();
		void Init(Repository *repo);
	protected:
		Repository *repo_;
	};
}; // namespace gitteh

#endif //GITTEH_GIT_OBJECT_H
