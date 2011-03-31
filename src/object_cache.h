#ifndef GITTEH_OBJECT_CACHE_H
#define GITTEH_OBJECT_CACHE_H

#include "gitteh.h"
#include <map>

namespace gitteh {

template <class P, class T, class S>
class CachedGitObject;

template<class P, class T, class S>
class WrappedGitObjectCache {
public:
	inline Handle<Value> syncCacheObject(S* obj) {
		HandleScope scope;
		T* wrappedObj = wrapObject(obj);

		if(!wrappedObj->isInitialized()) {
			int result = wrappedObj->waitForInitialization();

			std::cout << "res:"<<result<<"\n";
			if(result != GIT_SUCCESS) {
				THROW_GIT_ERROR("Couldn't get object.", result);
			}

			wrapWrappedObject(obj);
		}

		return scope.Close(Local<Object>::New(wrappedObj->handle_));
	}

	inline int asyncCacheObject(S* obj, T **wrappedObjRet) {
		T* wrappedObj = wrapObject(obj);

		int error = GIT_SUCCESS;
		if(!wrappedObj->isInitialized()) {
			error = wrappedObj->waitForInitialization();
			if(error == GIT_SUCCESS) {
				*wrappedObjRet = wrappedObj;
			}
		}

		return error;
	}

	void asyncEnsureWrapped(S *wrappedObj) {
		wrapWrappedObject(wrappedObj);
	}

	inline WrappedGitObjectCache(P *owner) {
		CREATE_MUTEX(objectsLock);
		owner_ = owner;
	}

	inline ~WrappedGitObjectCache() {
		LOCK_MUTEX(objectsLock);

		typename std::map<size_t, CachedGitObject<P, T,S>* >::const_iterator it = objects.begin();
		typename std::map<size_t, CachedGitObject<P, T,S>* >::const_iterator end = objects.end();
		CachedGitObject<P, T,S>* cachedObject;

		while(it != end) {
			cachedObject = it->second;

			if(cachedObject->wrapped) {
				cachedObject->handle.Dispose();
				cachedObject->handle.Clear();
			}

			delete cachedObject;
			++it;
		}

		UNLOCK_MUTEX(objectsLock);
	}

protected:
	// Returns the wrapping object for a git object. If there isn't one yet,
	// it'll create it.
	inline T* wrapObject(S* gitObj) {
		CachedGitObject<P, T, S> *cachedObject;
		bool newlyCreated = false;

		LOCK_MUTEX(objectsLock);
		cachedObject  = objects[(size_t)gitObj];

		if(cachedObject == NULL) {
			std::cout << "CREATING.\n";
			T* wrappedObject = new T(gitObj);
			wrappedObject->setOwner(owner_);

			newlyCreated = true;

			cachedObject = new CachedGitObject<P, T, S>();
			cachedObject->cache = this;
			cachedObject->wrapped = false;
			cachedObject->ref = gitObj;
			cachedObject->object = wrappedObject;

			objects[(size_t)gitObj] = cachedObject;
		}

		UNLOCK_MUTEX(objectsLock);

		return cachedObject->object;
	}

	// Hurr.
	inline void wrapWrappedObject(S* gitObj) {
		LOCK_MUTEX(objectsLock);
		CachedGitObject<P, T, S> *cachedObject = objects[(size_t)gitObj];
		cachedObject->object->ensureInitDone();
		cachedObject->handle = Persistent<Object>::New(cachedObject->object->handle_);
		cachedObject->handle.MakeWeak(cachedObject, WeakCallback);
		UNLOCK_MUTEX(objectsLock);
	}

/*
	inline bool getObjectFor (S *ref, T **dest) {
		HandleScope scope;

		CachedGitObject<P, T, S> *cachedObject;
		bool newlyCreated = false;

		// Quick check for the managed object.
		LOCK_MUTEX(objectsLock);
		cachedObject  = objects[(size_t)ref];

		if(managedObject == NULL) {
			// K we gotta go ahead and create.
			Handle<Value> constructorArgs[1] = { External::New(ref) };
			Handle<Object> jsObject = T::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

			managedObject = new ManagedObject<T, S>();
			managedObject->store = this;
			managedObject->object = ObjectWrap::Unwrap<T>(jsObject);
			managedObject->ref = ref;



			objects[(size_t)ref] = managedObject;

			newlyCreated = true;
		}

		*dest = managedObject->object;
		UNLOCK_MUTEX(objectsLock);

		return newlyCreated;
	}*/

private:
	static void WeakCallback (Persistent<Value> value, void *data) {
		CachedGitObject<P, T, S> *cachedObject = (CachedGitObject<P, T, S>*)data;

		assert(cachedObject->handle.IsNearDeath());

		WrappedGitObjectCache<P, T, S> *cache = cachedObject->cache;

		LOCK_MUTEX(cache->objectsLock);

		typename std::map<size_t, CachedGitObject<P, T,S>* >::iterator it;
		it = cache->objects.find((size_t)cachedObject->ref);
		cache->objects.erase(it);

		cachedObject->handle.Dispose();
		cachedObject->handle.Clear();

		UNLOCK_MUTEX(cache->objectsLock);
	}

	std::map<size_t, CachedGitObject<P, T, S>*> objects;
	gitteh_lock objectsLock;
	P *owner_;
};

template <class P, class T, class S>
class CachedGitObject {
public:
	WrappedGitObjectCache<P, T, S> *cache;
	T *object;
	S *ref;
	Persistent<Object> handle;
	bool wrapped;	// Is the object wrapped in a JS object yet?
};

} // namespace gitteh

#endif // GITTEH_OBJECT_CACHE_H
