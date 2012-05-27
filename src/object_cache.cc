#include "object_cache.h"
#include "git_object.h"
#include "commit.h"
#include "tree.h"
#include "blob.h"

// Implement < operator for git_oid so we can use it as key in STL map.
bool operator <(const git_oid l, const git_oid r) {
	return git_oid_cmp(&l, &r) < 0;
}

namespace gitteh {
	Handle<Value> GitObjectCache::wrap(git_object *obj, GitObject **out) {
		HandleScope scope;

		const git_oid *oid = git_object_id(obj);
		GitObject *wrappedObj;

		if(cache_.count(*oid)) {
			// return cache_[*oid];
			std::cout << "Cached VICTORY!" << std::endl;
			*out = cache_[*oid];
			return scope.Close(Local<Object>::New((*out)->handle_));
		}

		git_otype type = git_object_type(obj);

		Handle<Function> constructor;
		switch(type) {
			case GIT_OBJ_COMMIT: {
				wrappedObj = new Commit((git_commit*)obj);
				constructor = Commit::constructor_template->GetFunction();
				break;
			}
			case GIT_OBJ_TREE: {
				wrappedObj = new Tree((git_tree*)obj);
				constructor = Tree::constructor_template->GetFunction();
				break;
			}
			case GIT_OBJ_BLOB: {
				wrappedObj = new Blob((git_blob*)obj);
				constructor = Blob::constructor_template->GetFunction();
			}
			default: {
				assert(0);
			}
		}

		wrappedObj->Init(repo_);

		TryCatch tryCatch;
		Handle<Value> constructorArgs[] = { External::New(wrappedObj) };
		constructor->NewInstance(1, constructorArgs);
		wrappedObj->Ref();

		if(tryCatch.HasCaught()) {
			FatalException(tryCatch);
			return Undefined();
			// return NULL;
		}

		git_oid oidKey;
		git_oid_cpy(&oidKey, oid);
		cache_.insert(std::pair<git_oid, GitObject*>(oidKey, wrappedObj));

		*out = wrappedObj;
		return scope.Close(Local<Object>::New(wrappedObj->handle_));
	}

	void GitObjectCache::evict(GitObject *obj) {
		std::cout << "erase" << std::endl;
		cache_.erase(obj->oid_);
	}

}; // namespace gitteh
