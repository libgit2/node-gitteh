#include "git_object.h"
#include "commit.h"


namespace gitteh {
	GitObject::GitObject(git_object *obj) {
		type_ = git_object_type(obj);
		git_oid_cpy(&oid_, git_object_id(obj));
	}

	GitObject::~GitObject() {
		repo_->disown(this);
	}

	void GitObject::Init(Repository *repo) {
		repo_ = repo;
		repo_->adopt(this);
	}	
}; // namespace gitteh
