#ifndef GITTEH_WRAPPED_GIT_OBJECT_H
#define GITTEH_WRAPPED_GIT_OBJECT_H

#include "gitteh.h"
#include "object_cache.h"
#include <errno.h>

namespace gitteh {

template<class T, class S>
class WrappedGitObject : public ObjectWrap {
public:
	WrappedGitObject() {
		CREATE_MUTEX(gatekeeperLock_);
		initialized_ = false;
		initializing_ = false;
		initInterest_ = 0;
		initResult_ = GIT_ERROR;
	}

	~WrappedGitObject() {
		DESTROY_MUTEX(gatekeeperLock_);
	}

	inline void setCache(WrappedGitObjectCache<T, S> *cache) {
		cache_ = cache;
	}

	bool isInitialized() {
		LOCK_MUTEX(gatekeeperLock_);
		bool isInitialized = initialized_;
		UNLOCK_MUTEX(gatekeeperLock_);

		return isInitialized;
	}

	inline void registerInitInterest() {
		LOCK_MUTEX(gatekeeperLock_);
		initInterest_++;
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	inline int initialize(S *gitObj) {
		LOCK_MUTEX(gatekeeperLock_);
		/*if(initialized_) {
			UNLOCK_MUTEX(gatekeeperLock_);
			return initResult_;
		}*/

		bool shouldInitialize = !initializing_;
		if(shouldInitialize) {
			initializing_ = true;
			CREATE_MUTEX(initLock_);
			LOCK_MUTEX(initLock_);
		}

		UNLOCK_MUTEX(gatekeeperLock_);

		if(shouldInitialize) {
			initResult_ = doInit();
			if(initResult_ != GIT_SUCCESS) {
				cache_->remove(gitObj);
			}

			LOCK_MUTEX(gatekeeperLock_);
			initialized_ = true;
			UNLOCK_MUTEX(gatekeeperLock_);

			UNLOCK_MUTEX(initLock_);
		}
		else {
			while(pthread_mutex_trylock(&initLock_) == EBUSY) {
				usleep(1000);
			}
			UNLOCK_MUTEX(initLock_);
		}

		LOCK_MUTEX(gatekeeperLock_);
		bool killInitLock = !(--initInterest_);
		UNLOCK_MUTEX(gatekeeperLock_);

		if(killInitLock) {
			if(initResult_ != GIT_SUCCESS) {
				// We're the last one out the door on a botched initialization
				// attempt.
				delete this;
			}
			DESTROY_MUTEX(initLock_);
		}

		return initResult_;
	}

	// Ensure this object is wrapped in a JS object.
	inline void ensureWrapped() {
		HandleScope scope;

		if(handle_.IsEmpty()) {
			Handle<Value> constructorArgs[1] = { External::New(this) };
			T::constructor_template->GetFunction()->NewInstance(1, constructorArgs);
			cache_->objectWrapped((T*)this);
		}

		cache_->unrefWrapped((T*)this);
	}

	inline void dontDieOnMeNow() {
		Ref();
		Unref();
	}

protected:
	virtual int doInit() = 0;

private:
	gitteh_lock gatekeeperLock_;

	gitteh_lock initLock_;
	bool initialized_;
	bool initializing_;
	int initResult_;
	int initInterest_;

	WrappedGitObjectCache<T, S> *cache_;
};

} // namespace gitteh

#endif // GITTEH_WRAPPED_GIT_OBJECT_H
