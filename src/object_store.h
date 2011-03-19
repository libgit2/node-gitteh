#ifndef GITTEH_OBJ_STORE_H
#define GITTEH_OBJ_STORE_H

#include "gitteh.h"
#include <map>

namespace gitteh {

// Oh man, what a mess this is. Being a C++ retard I hacked this up in the only
// way I knew how, repeated proddings at the syntax until the compiler finally
// gave up and realized I wasn't going to stop throwing ridiculous nonsensical
// code at it until it gave me what I demanded. Basically what this system does
// is allocates Javascript objects for underlying pointers. If I create one of
// these ObjectStores for, say, a git_commit*, I can then throw git_commit*
// pointers at it, and it will either create a neat new Commit object to wrap
// that underlying pointer. If I was to throw the same git_commit pointer at it
// later, it would return me the same JS object. The other nifty thing this lib
// does is maintain it's OWN Persistent handle to the JS object, also making it
// weak. The cool thing about this is if one of the wrapped objects gets reaped
// by the constructor, Node's ObjectWrap weakref will handle the actual deletion
// of the object, whereas my weak ref will just handle deleting the object from
// the backing map. This ensures future requests to wrap a pointer (even if it
// was already wrapped before V8 GC went and trashed the object) will then
// re-create the JS object. Woot.

// Restating what is probably obvious to those who know what they're doing (not
// me): there's no need to make any of this thread safe, nor will there ever be.
// This is because any wrapping/v8 object instantiation should only be done in 
// the main thread.

// Revision to the above statement: I've added locks to accessing the map
// anyway. This is because I want to optimize the way I'm loading things later,
// and only load all the relevant git object data into a struct if it hasn't
// been loaded into an object here already.

template <class T, class S>
class ManagedObject;

template <class T, class S>
class ObjectStore {
public:
	inline bool getObjectFor (S *ref, T **dest) {
		HandleScope scope;

		ManagedObject<T, S> *managedObject;
		bool newlyCreated = false;

		// Quick check for the managed object.
		LOCK_MUTEX(objectsLock);
		managedObject  = objects[(int)ref];
		UNLOCK_MUTEX(objectsLock);

		if(managedObject == NULL) {
			// K we gotta go ahead and create.
			Handle<Value> constructorArgs[1] = { External::New(ref) };
			Handle<Object> jsObject = T::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

			managedObject = new ManagedObject<T, S>();
			managedObject->store = this;
			managedObject->object = ObjectWrap::Unwrap<T>(jsObject);
			managedObject->ref = ref;

			managedObject->handle = Persistent<Object>::New(managedObject->object->handle_);
			managedObject->handle.MakeWeak(managedObject, WeakCallback);

			LOCK_MUTEX(objectsLock);
			objects[(int)ref] = managedObject;
			UNLOCK_MUTEX(objectsLock);

			newlyCreated = true;
		}

		//return scope.Close(managedObject->object->handle_);
		*dest = managedObject->object;

		return newlyCreated;
	}

	inline void deleteObjectFor(S* ref) {
		ManagedObject<T, S> *managedObject;

		LOCK_MUTEX(objectsLock);
		managedObject = objects[(int)ref];
		if(!managedObject) return;

		managedObject->handle.ClearWeak();
		managedObject->handle.Dispose();

		typename std::map<int, ManagedObject<T,S>* >::iterator it;
		it = objects.find((int)ref);
		objects.erase(it);
		UNLOCK_MUTEX(objectsLock);
	}

	inline bool objectExistsFor(S* ref) {
		bool exists = false;
		LOCK_MUTEX(objectsLock);
		if(objects[(int)ref] != NULL) exists = true;
		UNLOCK_MUTEX(objectsLock);

		return exists;
	}

	inline ObjectStore() {
		CREATE_MUTEX(objectsLock);
	}

	inline ~ObjectStore() {
		// TODO: ooh, could be all sorts of fucked up deadlocks that locking the
		// mutex in a dtor might bring about. Gonna have to make sure this is ok
		// ... Later. Probably after another day of debugging stupid shit.
		LOCK_MUTEX(objectsLock);

		typename std::map<int, ManagedObject<T,S>* >::const_iterator it = objects.begin();
		typename std::map<int, ManagedObject<T,S>* >::const_iterator end = objects.end();
		ManagedObject<T,S>* managedObject;

		while(it != end) {
			managedObject = it->second;
			managedObject->handle.Dispose();
			managedObject->handle.Clear();
			delete managedObject;
			++it;
		}

		UNLOCK_MUTEX(objectsLock);
	}

private:
	static void WeakCallback (Persistent<Value> value, void *data) {
		ManagedObject<T, S> *managedObject = (ManagedObject<T, S>*)data;

		assert(managedObject->handle.IsNearDeath());

		ObjectStore<T, S> *store = managedObject->store;


		LOCK_MUTEX(store->objectsLock);

		typename std::map<int, ManagedObject<T,S>* >::iterator it;
		it = store->objects.find((int)managedObject->ref);
		store->objects.erase(it);

		managedObject->handle.Dispose();
		managedObject->handle.Clear();

		UNLOCK_MUTEX(store->objectsLock);
	}

	std::map<int, ManagedObject<T, S>*> objects;
	gitteh_lock objectsLock;
};

template <class T, class S>
class ManagedObject {
public:
	ObjectStore<T, S> *store;
	T *object;
	S *ref;
	Persistent<Object> handle;
};

} // namespace gitteh

#endif	// GITTEH_OBJ_STORE_H
