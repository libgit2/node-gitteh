#include "object_factory.h"
#include "repository.h"

ObjectFactory::ObjectFactory(Repository *repo) {
	//repo_ = repo;
}

ObjectFactory::~ObjectFactory() {

}

void ObjectFactory::asyncRequestObject(T *gitObject, Persistent<Function>& callback) {
	S *object = wrap(gitObject);

	if(object->isInitialized()) {
		ReturnWrappedObject(object, callback);
		return;
	}

	build_object_request *req = new build_commit_request<T,S>;
	req->callback = callback;
	req->gitObject = object;
	req->jsObject = commitObject;

	object->registerInitInterest();
	repo_->Ref();
	eio_custom(EIO_BuildObject, EIO_PRI_DEFAULT, EIO_ReturnBuiltObject, req);
	ev_ref(EV_DEFAULT_UC);
}

S ObjectFactory::*wrap(T *gitObj) {

}
