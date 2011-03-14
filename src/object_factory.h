#ifndef GITTEH_OBJECT_FACTORY_H
#define GITTEH_OBJECT_FACTORY_H

#include "object_store.h"
#include "repository.h"

namespace gitteh {

class Repository;

template<class T, class S>
struct build_object_request {
	Persistent<Function> callback;
	ObjectFactory<T,S> *factory;
	S *gitObject;
	T *jsObject;
};

template<class T, class S>
class ObjectFactory {
public:
	ObjectFactory(Repository *repo) {
		repo_ = repo;
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

		build_object_request<T, S> *req = new build_object_request<T, S>;
		req->callback = callback;
		req->gitObject = gitObject;
		req->jsObject = object;
		req->factory = this;

		object->registerInitInterest();
		repo_->Ref();
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

protected:
	inline T *wrap(S *gitObj) {
		T *object;

		if(store_.getObjectFor(gitObj, &object)) {
			// Commit needs to know who it's daddy is.
			object->repository_ = repo_;
		}

		return object;
	}

private:
	static int EIO_BuildObject(eio_req *req) {
		build_object_request<T,S> *reqData = static_cast<build_object_request<T,S>*>(req->data);
		reqData->jsObject->waitForInitialization();
		return 0;
	}

	static int EIO_ReturnBuiltObject(eio_req *req) {
		build_object_request<T,S> *reqData = static_cast<build_object_request<T,S>*>(req->data);

		ev_unref(EV_DEFAULT_UC);
		reqData->factory->repo_->Unref();

		reqData->jsObject->ensureInitDone();
		reqData->jsObject->removeInitInterest();
		ReturnWrappedObject(reqData->jsObject, reqData->callback);
		delete reqData;

		return 0;
	}

	static inline void ReturnWrappedObject(ThreadSafeObjectWrap *obj,
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

	Repository *repo_;
	ObjectStore<T,S> store_;
};

} // namespace gitteh

#endif // GITTEH_OBJECT_FACTORY_H
