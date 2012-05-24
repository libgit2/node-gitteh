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
		git_otype type_;
		git_oid oid_;

		GitObject(git_object *obj) {
			type_ = git_object_type(obj);
			git_oid_cpy(&oid_, git_object_id(obj));
		}

		~GitObject() {
			repo_->disown(this);
		}

		inline void Init(Repository *repo) {
			repo_ = repo;
			repo_->adopt(this);
		}

	protected:
		Repository *repo_;
	};
};

#endif //GITTEH_GIT_OBJECT_H