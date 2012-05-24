#ifndef GITTEH_OBJECT_CACHE_H_
#define GITTEH_OBJECT_CACHE_H_

#include <git2.h>
#include <map>

bool operator <(const git_oid l, const git_oid r);

namespace gitteh {
	class Repository;
	class GitObject;

	class GitObjectCache {
	public:
		GitObjectCache(Repository *repo) {
			repo_ = repo;
		}
		GitObject *wrap(git_object*);
		/**
		 * Rather than setting up our own weakref to the GitObjects we wrap, we
		 * instead wait for the Repository to inform us of GitObject deaths.
		 * These happen when Node's ObjectWrap weakref handler kicks in and
		 * deletes the object. GitObject destructor tells Repository it's dead,
		 * which then tells us. This might be a little convoluted, I dunno. I 
		 * did this because dying GitObjects need to tell Repository they're 
		 * dying anyway, so Repo can Unref() itself cleanly.
		*/
		void evict(GitObject*);
	private:
		Repository *repo_;
		std::map<git_oid, GitObject*> cache_;
	};
};

#endif // GITTEH_OBJECT_CACHE_H_
