#include "odb.h"

namespace gitteh {

static Persistent<String> odb_class_symbol;

Persistent<FunctionTemplate> ObjectDatabase::constructor_template;

struct open_request {
	Persistent<Function> callback;
	std::string *path;
	git_odb *odb;
	int error;
};

struct get_request {
	Persistent<Function> callback;
	ObjectDatabase *odb;
	git_oid oid;
	git_odb_object *obj;
	int error;
};

void ObjectDatabase::Init(Handle<Object> target) {
	HandleScope scope;

	odb_class_symbol = NODE_PSYMBOL("ObjectDatabase");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(odb_class_symbol);
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);

	target->Set(odb_class_symbol, constructor_template->GetFunction());
	NODE_SET_METHOD(target, "openODB", Open);
}

Handle<Value> ObjectDatabase::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(2);
	REQ_EXT_ARG(0, odbArg);
	REQ_BOOL_ARG(1, createdArg);

	ObjectDatabase *odb = new ObjectDatabase();
	odb->Wrap(args.This());

	odb->odb_ = static_cast<git_odb*>(odbArg->Value());
	odb->created_ = createdArg;

	return args.This();
}

Handle<Value> ObjectDatabase::Open(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_STR_ARG(0, pathArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		open_request *request = new open_request;
		request->callback = Persistent<Function>::New(callbackArg);
		request->path = new std::string(*pathArg);

		eio_custom(EIO_Open, EIO_PRI_DEFAULT, EIO_AfterOpen, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_odb *odb;
		int result = git_odb_open(&odb, *pathArg);

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't open ODB.", result);
		}

		Handle<Value> constructorArgs[2] = {
			External::New(odb),
			Boolean::New(true)
		};

		return scope.Close(ObjectDatabase::constructor_template->GetFunction()
				->NewInstance(2, constructorArgs));
	}
}

int ObjectDatabase::EIO_Open(eio_req *req) {
	open_request *reqData = static_cast<open_request*>(req->data);

	reqData->error = git_odb_open(&reqData->odb, reqData->path->c_str());

	delete reqData->path;
	return 0;
}

int ObjectDatabase::EIO_AfterOpen(eio_req *req) {
	HandleScope scope;
	open_request *reqData = static_cast<open_request*>(req->data);

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_SUCCESS) {
		Handle<Value> error = CreateGitError(String::New("Couldn't open ODB."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Handle<Value> constructorArgs[2] = {
			External::New(reqData->odb),
			Boolean::New(true)
		};

		callbackArgs[0] = Undefined();
		callbackArgs[1] = ObjectDatabase::constructor_template->GetFunction()
								->NewInstance(2, constructorArgs);
	}

	TRIGGER_CALLBACK();

	reqData->callback.Dispose();
	delete reqData;
	ev_unref(EV_DEFAULT_UC);
	return 0;
}

Handle<Value> ObjectDatabase::Exists(const Arguments& args) {
	HandleScope scope;
	ObjectDatabase *odb = ObjectWrap::Unwrap<ObjectDatabase>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		THROW_ERROR("Unimplemented.");
	}
	else {
		odb->lockOdb();
		bool result = git_odb_exists(odb->odb_, &oidArg);
		odb->unlockOdb();

		return scope.Close(Boolean::New(result));
	}
}

Handle<Value> ObjectDatabase::Get(const Arguments& args) {
	HandleScope scope;
	ObjectDatabase *odb = ObjectWrap::Unwrap<ObjectDatabase>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		THROW_ERROR("Unimplemented.");
	}
	else {
		odb->lockOdb();

		odb->unlockOdb();
	}
}

int ObjectDatabase::EIO_Get(eio_req *req) {

}

int ObjectDatabase::EIO_AfterGet(eio_req *req) {
	HandleScope scope;
}

ObjectDatabase::ObjectDatabase() {
	objectCache_ = new WrappedGitObjectCache<ObjectDatabase, ODBObject, git_odb_object>(this);
}

ObjectDatabase::~ObjectDatabase() {
	if(created_) {
		git_odb_close(odb_);
	}
}

} // namespace gitteh
