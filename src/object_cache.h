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

	~WrappedGitObjectCache() {
	}

	inline int asyncRequest(S *gitObject, T **wrappedObject) {
		return wrap(gitObject, wrappedObject);
	}

	inline Handle<Value> syncRequest(S *gitObject) {
		HandleScope scope;

		T *wrappedObject;
		int result = wrap(gitObject, &wrappedObject);
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't load object.", result);
		}

		wrappedObject->ensureWrapped();
		return scope.Close(Local<Object>::New(wrappedObject->handle_));
	}

	inline void remove(S *gitObject) {
		CachedObject<T, S> *cachedObject;

		LOCK_MUTEX(objectsLock_);
		cachedObject = objects_[(size_t)gitObject];
		if(cachedObject) {
			if(!cachedObject->handle.IsEmpty()) {
				cachedObject->handle.ClearWeak();
				cachedObject->handle.Dispose();
			}

			typename std::map<size_t, CachedObject<T,S>* >::iterator it;
			it = objects_.find((size_t)gitObject);
			objects_.erase(it);

			delete cachedObject;
		}

		UNLOCK_MUTEX(objectsLock_);
	}

	inline void objectWrapped(T *wrappedObject) {
		LOCK_MUTEX(objectsLock_);
		CachedObject<T, S> *cachedObject = objectsByWrapped_[wrappedObject];
		cachedObject->handle = Persistent<Object>::New(wrappedObject->handle_);
		cachedObject->handle.MakeWeak(cachedObject, WeakCallback);
		UNLOCK_MUTEX(objectsLock_);
	}

	inline void unrefWrapped(T *wrappedObject) {
		LOCK_MUTEX(objectsLock_);
		CachedObject<T, S> *cachedObject = objectsByWrapped_[wrappedObject];
		cachedObject->references--;
		UNLOCK_MUTEX(objectsLock_);
	}

	inline T **getAllObjects(int *countRet) {
		LOCK_MUTEX(objectsLock_);
		int count = *countRet = objects_.size();

		typename std::map<size_t, CachedObject<T,S>* >::const_iterator it = objects_.begin();
		typename std::map<size_t, CachedObject<T,S>* >::const_iterator end = objects_.end();
		CachedObject<T,S>* cachedObject;

		T **objectPtrs = new T*[count];
		int i = 0;
		while(it != end) {
			cachedObject = it->second;
			objectPtrs[i++] = cachedObject->object;
			++it;
		}

		UNLOCK_MUTEX(objectsLock_);
		return objectPtrs;
	}

	// Currently used to inform the cache to update its internal store when
	// refs are packed, this is because ref packing invalidates old pointer and
	// a new one is created, however it's still the same object so no need to
	// re-wrap etc.
	inline void updateCacheRef(S *oldRef, S *newRef) {
		LOCK_MUTEX(objectsLock_);

		// Grab the current cache entry.
		CachedObject<T, S> *cachedObject = objects_[(size_t)oldRef];

		// Delete the cache entry from store pointed to by old ref.
		typename std::map<size_t, CachedObject<T,S>* >::iterator it;
		it = objects_.find((size_t)oldRef);
		objects_.erase(it);

		// Reinsert with new ref.
		objects_[(size_t)newRef] = cachedObject;

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
			wrappedObject->setOwner(owner_);

			cachedObject = new CachedObject<T, S>;
			cachedObject->cache = this;
			cachedObject->object = wrappedObject;
			cachedObject->ref = gitObject;
			cachedObject->references = 0;

			objects_[(size_t)gitObject] = cachedObject;
			objectsByWrapped_[wrappedObject] = cachedObject;
		}

		bool shouldInitialize = !cachedObject->object->isInitialized();
		if(shouldInitialize) {
			cachedObject->object->registerInitInterest();
		}

		// Make sure WeakReference knows to keep this object alive. This
		// should be thread safe, since the cachedobject is protected by the
		// object lock. Easiest way would just be to call Ref() on the
		// wrapping object, but Ref() will in turn call ClearWeak, which I
		// doubt can be used in threads.
		cachedObject->references++;

		UNLOCK_MUTEX(objectsLock_);

		if(shouldInitialize) {
			result = cachedObject->object->initialize(gitObject);
		}
		else {
			result = GIT_OK;
		}

		if(result == GIT_OK) {
			*object = cachedObject->object;
		}
		return result;
	}

	static void WeakCallback (Persistent<Value> value, void *data) {
		CachedObject<T, S> *cachedObject = static_cast<CachedObject<T, S>*>(data);

		assert(cachedObject->handle.IsNearDeath());

		WrappedGitObjectCache<T, S> *cache = cachedObject->cache;

		LOCK_MUTEX(cache->objectsLock_);

		// Check if there's any asynchronous interest in this object.
		if(cachedObject->references) {
			// Well shite. There is. Revive the object and exit.
			cachedObject->object->dontDieOnMeNow();

			value.MakeWeak(cachedObject, WeakCallback);
			UNLOCK_MUTEX(cache->objectsLock_);

			return;
		}

		typename std::map<size_t, CachedObject<T,S>* >::iterator it;
		it = cache->objects_.find((size_t)cachedObject->ref);
		cache->objects_.erase(it);

		typename std::map<T*, CachedObject<T,S>* >::iterator alsoIt;
		alsoIt = cache->objectsByWrapped_.find(cachedObject->object);
		cache->objectsByWrapped_.erase(alsoIt);

		cachedObject->handle.Dispose();
		cachedObject->handle.Clear();

		//cachedObject->object->timeToDie();
		UNLOCK_MUTEX(cache->objectsLock_);
	}

	Repository *owner_;
	std::map<size_t, CachedObject<T, S>*> objects_;
	std::map<T*, CachedObject<T, S>*> objectsByWrapped_;
	gitteh_lock objectsLock_;
};

template <class T, class S>
class CachedObject {
public:
	WrappedGitObjectCache<T, S> *cache;
	T *object;
	S *ref;
	Persistent<Object> handle;
	int references;
};

} // namespace gitteh

#endif // GITTEH_OBJECT_CACHE_H_
