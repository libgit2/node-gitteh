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

template <class T, class S>
class ManagedObject;

template <class T, class S>
class ObjectStore {
public:
	inline bool getObjectFor (S *ref, T **dest) {
		HandleScope scope;

		ManagedObject<T, S> *managedObject;
		bool newlyCreated = false;

		if(!objects[(int)ref]) {
			Handle<Value> constructorArgs[1] = { External::New(ref) };
			Handle<Object> jsObject = T::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

			managedObject = new ManagedObject<T, S>();
			managedObject->store = this;
			managedObject->object = ObjectWrap::Unwrap<T>(jsObject);
			managedObject->ref = ref;
			objects[(int)ref] = managedObject;

			managedObject->handle = Persistent<Object>::New(managedObject->object->handle_);
			managedObject->handle.MakeWeak(managedObject, WeakCallback);

			newlyCreated = true;
		}
		else {
			managedObject = objects[(int)ref];
		}

		//return scope.Close(managedObject->object->handle_);
		*dest = managedObject->object;
		return newlyCreated;
	}

	inline void deleteObjectFor(S* ref) {
		ManagedObject<T, S> *managedObject;

		managedObject = objects[(int)ref];
		if(!managedObject) return;

		managedObject->handle.ClearWeak();
		managedObject->handle.Dispose();

		typename std::map<int, ManagedObject<T,S>* >::iterator it;
		it = objects.find((int)ref);
		objects.erase(it);
	}

	inline ~ObjectStore() {
		typename std::map<int, ManagedObject<T,S>* >::const_iterator it = objects.begin();
		typename std::map<int, ManagedObject<T,S>* >::const_iterator end = objects.end();
		ManagedObject<T,S>* managedObject;

		while(it != end) {
			managedObject = it->second;
			managedObject->handle.ClearWeak();
			managedObject->handle.Dispose();
			++it;
		}
	}

private:
	static void WeakCallback (Persistent<Value> value, void *data) {
		ManagedObject<T, S> *managedObject = (ManagedObject<T, S>*)data;
		ObjectStore<T, S> *store = managedObject->store;

		typename std::map<int, ManagedObject<T,S>* >::iterator it;
		it = store->objects.find((int)managedObject->ref);
		store->objects.erase(it);
	}

	std::map<int, ManagedObject<T, S>*> objects;
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
