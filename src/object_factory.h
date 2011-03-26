#ifndef GITTEH_OBJECT_FACTORY_H
#define GITTEH_OBJECT_FACTORY_H

#include "object_store.h"
#include "repository.h"

namespace gitteh {

class Repository;

template<class P, class T, class S>
struct build_object_request {
	Persistent<Function> callback;
	ObjectFactory<P,T,S> *factory;
	S *gitObject;
	T *jsObject;
};

template<class P, class T, class S>
class ObjectFactory {
public:
	ObjectFactory(P *owner) {
		owner_ = owner;
	}

	~ObjectFactory() {
	}

	inline T *newObject(S *gitObject) {
		T *object = wrap(gitObject);
		object->forceInitialized();
		return object;
	}

	inline void asyncRequestObject(S *gitObject, Persistent<Function>& callback) {
		T *object = wrap(gitObject);

		if(object->isInitialized()) {
			ReturnWrappedObject(object, callback);
			return;
		}

		build_object_request<P, T, S> *req = new build_object_request<P, T, S>;
		req->callback = callback;
		req->gitObject = gitObject;
		req->jsObject = object;
		req->factory = this;

		object->registerInitInterest();
		owner_->Ref();
		eio_custom(EIO_BuildObject, EIO_PRI_DEFAULT, EIO_ReturnBuiltObject, req);
		ev_ref(EV_DEFAULT_UC);
	}

	inline T *syncRequestObject(S *gitObject) {
		T *object = wrap(gitObject);

		if(!object->isInitialized()) {
			object->registerInitInterest();
			object->waitForInitialization();
			object->removeInitInterest();
			object->ensureInitDone();
		}

		return object;
	}

	inline void deleteObject(S *gitObject) {
		store_.deleteObjectFor(gitObject);
	}

protected:
	inline T *wrap(S *gitObj) {
		T *object;

		if(store_.getObjectFor(gitObj, &object)) {
			//object->repository_ = static_cast<Repository*>(owner_);

			// TODO: My plan here is that when we initialize an object, we inc
			// the ref counter on the object that created it. That way, if we
			// had a situation where someone opened a repo, then loaded a commit
			// but lost reference to the repo, the repo would hang around in
			// memory until the commit reference is lost also. To make this work
			// I need some way to *hook* into the Unref(), most likely via a
			// friend class (which I've started), but I need a hook into the
			// ObjectStore to catch when the weakref callback is hit, or the
			// object is explicitly removed, then I can Unref() from here. I
			// think I'll end up merging ObjectStore and ObjectFactory soon, so
			// I'll do it then.
			//owner_->Ref();
			object->setOwner(owner_);
		}

		return object;
	}

private:
	static int EIO_BuildObject(eio_req *req) {
		build_object_request<P,T,S> *reqData = static_cast<build_object_request<P,T,S>*>(req->data);
		reqData->jsObject->waitForInitialization();
		return 0;
	}

	static int EIO_ReturnBuiltObject(eio_req *req) {
		build_object_request<P,T,S> *reqData = static_cast<build_object_request<P,T,S>*>(req->data);

		reqData->jsObject->ensureInitDone();
		reqData->jsObject->removeInitInterest();

		ev_unref(EV_DEFAULT_UC);
		reqData->factory->owner_->Unref();

		ReturnWrappedObject(reqData->jsObject, reqData->callback);
		delete reqData;

		return 0;
	}

	static inline void ReturnWrappedObject(GitObjectWrap *obj,
		Persistent<Function>& callback) {
		Handle<Value> callbackArgs[2];
		callbackArgs[0] = Null();
		callbackArgs[1] = obj->handle_;

		TryCatch tryCatch;
		callback->Call(Context::GetCurrent()->Global(), 2, callbackArgs);

	 	if(tryCatch.HasCaught()) {
	       FatalException(tryCatch);
	    }

	 	CLEANUP_CALLBACK(callback);
	}

	P *owner_;
	ObjectStore<T,S> store_;
};

} // namespace gitteh

#endif // GITTEH_OBJECT_FACTORY_H
