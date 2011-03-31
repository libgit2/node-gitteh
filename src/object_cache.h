#ifndef GITTEH_OBJECT_CACHE_H_
#define GITTEH_OBJECT_CACHE_H_

#include "gitteh.h"
#include <map>

namespace gitteh {

template<class T, class S> class CachedObject;

class Repository;

template<class T, class S>
class WrappedGitObjectCache {
public:
	WrappedGitObjectCache(Repository *owner) {
		owner_ = owner;
		CREATE_MUTEX(objectsLock_);
	}

	inline int asyncRequest(S *gitObject, T **wrappedObject) {
		return wrap(gitObject, wrappedObject);
	}

	inline int syncRequest(S *gitObject, T **wrappedObject) {
		wrap(gitObject, wrappedObject);
	}

	inline void remove(S *gitObject) {
		CachedObject<T, S> *cachedObject;

		LOCK_MUTEX(objectsLock_);
		cachedObject = objects_[(size_t)gitObject];
		if(cachedObject) {
			if(cachedObject->isJsWrapped) {
				cachedObject->handle.ClearWeak();
				cachedObject->handle.Dispose();
			}

			typename std::map<size_t, CachedObject<T,S>* >::iterator it;
			it = objects_.find((size_t)gitObject);
			objects_.erase(it);
		}

		UNLOCK_MUTEX(objectsLock_);
	}

private:
	inline int wrap(S *gitObject, T **object) {
		int result;

		LOCK_MUTEX(objectsLock_);
		CachedObject<T, S> *cachedObject = objects_[(size_t)gitObject];

		if(cachedObject == NULL) {
			T *wrappedObject = new T(gitObject);
			wrappedObject->setCache(this);

			cachedObject = new CachedObject<T, S>;
			cachedObject->cache = this;
			cachedObject->object = wrappedObject;
			cachedObject->ref = gitObject;
			cachedObject->isJsWrapped = false;
			objects_[(size_t)gitObject] = cachedObject;
		}

		bool shouldInitialize = !cachedObject->wrappedObject->isInitialized();
		if(shouldInitialize) {
			cachedObject->object->registerInitInterest();
		}
		UNLOCK_MUTEX(objectsLock_);

		if(shouldInitialize) {
			result = cachedObject->object->initialize(gitObject);
		}
		else {
			result = GIT_SUCCESS;
		}

		if(result == GIT_SUCCESS) {
			*object = cachedObject->object;
		}
		return result;
	}

	Repository *owner_;
	std::map<size_t, CachedObject<T, S>*> objects_;
	gitteh_lock objectsLock_;
};

template <class T, class S>
class CachedObject {
public:
	WrappedGitObjectCache<T, S> *cache;
	T *object;
	S *ref;
	Persistent<Object> handle;
	bool isJsWrapped;
};

} // namespace gitteh

#endif // GITTEH_OBJECT_CACHE_H_
