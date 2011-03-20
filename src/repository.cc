/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "repository.h"
#include "commit.h"
#include "tree.h"
#include "index.h"
#include "tag.h"
#include "rev_walker.h"
#include "rawobj.h"
#include "ref.h"
#include "object_factory.h"

// DANGER, WILL ROBINSON!
// The nastiest code that will ever rape your eyeballs follows.
// While writing async callbacks for getting/creating all the different objects
// I decided that no, I didn't want to copy and paste the same blocks of code
// 6 trillion times. Instead, I write a bunch of ugly motherfucking macros that
// I have no hope of maintaining in future. Fuck I'm rad.

#define GET_REQUEST_DATA(REQUESTTYPE)										\
	REQUESTTYPE *reqData =													\
		static_cast<REQUESTTYPE*>(req->data);

#define REQUEST_CLEANUP()													\
    reqData->callback.Dispose();											\
 	ev_unref(EV_DEFAULT_UC);												\
 	reqData->repo->Unref();													\
 	delete reqData;															\
	return 0;

#define REQUEST_DETACH(OBJ, FN, AFTERFN)									\
	OBJ->Ref();																\
	eio_custom(FN, EIO_PRI_DEFAULT, AFTERFN, request);						\
	ev_ref(EV_DEFAULT_UC);													\
	return scope.Close(Undefined());

#define CREATE_ASYNC_REQUEST(REQUESTCLASS)									\
	REQUESTCLASS *request = new REQUESTCLASS();								\
	request->callback = Persistent<Function>::New(callbackArg);				\
	request->repo = repo;

#define SETUP_CALLBACK_ARGS(TYPE, CLASS)									\
		Handle<Value> callbackArgs[2];										\
	 	if(reqData->error) {												\
	 		Handle<Value> error = CreateGitError(String::New(				\
	 				"Couldn't get " # CLASS), reqData->error);				\
	 		callbackArgs[0] = error;										\
	 		callbackArgs[1] = Null();										\
		}																	\
		else {																\
			CLASS *object = reqData->repo->wrap##CLASS(						\
					static_cast<TYPE*>(reqData->object));					\
			callbackArgs[0] = Null();										\
			callbackArgs[1] = object->handle_;								\
		}


#define FN_ASYNC_GET_OID_OBJECT(TYPE, GIT_TYPE)								\
	int Repository::EIO_Get##TYPE(eio_req *req) {							\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE *object;													\
		reqData->error = reqData->repo->get##TYPE(&reqData->oid,			\
				&object);													\
		if(reqData->error == GIT_SUCCESS) {									\
			reqData->object = object;										\
		}																	\
		return 0;															\
	}

#define FN_ASYNC_GET_NAMED_OBJECT(TYPE, GIT_TYPE)							\
	int Repository::EIO_Get##TYPE(eio_req *req) {							\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE *object;													\
		reqData->error = reqData->repo->get##TYPE(reqData->name->c_str(),	\
				&object);													\
		if(reqData->error == GIT_SUCCESS) {									\
			reqData->object = object;										\
		}																	\
		delete reqData->name;												\
		return 0;															\
	}

#define FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(TYPE, GIT_TYPE, FACTORY)			\
	int Repository::EIO_Return##TYPE(eio_req *req) {						\
		HandleScope scope;													\
		GET_REQUEST_DATA(object_request);									\
		ev_unref(EV_DEFAULT_UC);											\
		reqData->repo->Unref();												\
		Handle<Value> callbackArgs[2];										\
		if(reqData->error != GIT_SUCCESS) {									\
			Handle<Value> error = CreateGitError(String::New(				\
					"Git error."), reqData->error);							\
			callbackArgs[0] = error;										\
			callbackArgs[1] = Null();										\
			TRIGGER_CALLBACK();												\
			reqData->callback.Dispose();									\
		}																	\
		else {																\
			if(reqData->create) {											\
				TYPE *obj = reqData->repo->FACTORY->newObject(				\
					static_cast<GIT_TYPE*>(reqData->object));				\
				callbackArgs[0] = Null();									\
				callbackArgs[1] = obj->handle_;								\
				TRIGGER_CALLBACK();											\
				reqData->callback.Dispose();								\
			}																\
			else {															\
				reqData->repo->FACTORY->asyncRequestObject(					\
						static_cast<GIT_TYPE*>(reqData->object),			\
						reqData->callback);									\
			}																\
		}																	\
		delete reqData;														\
		return 0;															\
	}

#define FN_ASYNC_RETURN_OBJECT_VIA_WRAP(TYPE, GIT_TYPE)						\
	int Repository::EIO_Return##TYPE(eio_req *req) {						\
		HandleScope scope;													\
		GET_REQUEST_DATA(object_request);									\
		ev_unref(EV_DEFAULT_UC);											\
		reqData->repo->Unref();												\
		Handle<Value> callbackArgs[2];										\
		if(reqData->error != GIT_SUCCESS) {									\
			Handle<Value> error = CreateGitError(String::New(				\
					"Git error."), reqData->error);							\
			callbackArgs[0] = error;										\
			callbackArgs[1] = Null();										\
		}																	\
		else {																\
			TYPE *obj = reqData->repo->wrap##TYPE(							\
					static_cast<GIT_TYPE*>(reqData->object));				\
			callbackArgs[0] = Null();										\
			callbackArgs[1] = obj->handle_;									\
		}																	\
		TRIGGER_CALLBACK();													\
		reqData->callback.Dispose();										\
		delete reqData;														\
		return 0;															\
	}

#define FN_ASYNC_CREATE_OBJECT(TYPE, GIT_TYPE)								\
	int Repository::EIO_Create##TYPE(eio_req *req) {						\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE* object;													\
		reqData->error = reqData->repo->create##TYPE(&object);				\
		if(reqData->error == GIT_SUCCESS) {									\
			reqData->object = object;										\
		}																	\
		return 0;															\
	}

#define ASYNC_PREPARE_GET_OID_OBJECT(TYPE, GIT_TYPE)						\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	memcpy(&request->oid, &oidArg, sizeof(git_oid));						\
	REQUEST_DETACH(repo, EIO_Get##TYPE, EIO_Return##TYPE);

#define ASYNC_PREPARE_GET_NAMED_OBJECT(TYPE, GIT_TYPE)						\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	request->name = new std::string(*nameArg);								\
	REQUEST_DETACH(repo, EIO_Get##TYPE, EIO_Return##TYPE);

#define ASYNC_PREPARE_CREATE_OBJECT(TYPE)									\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	request->create = true;													\
	REQUEST_DETACH(repo, EIO_Create##TYPE, EIO_Return##TYPE);

#define SYNC_GET_OID_OBJECT(TYPE, GIT_TYPE, FACTORY)						\
	GIT_TYPE *object;														\
	int res = repo->get##TYPE(&oidArg, &object);							\
	if(res != GIT_SUCCESS) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			syncRequestObject(object)->handle_);

#define SYNC_GET_NAMED_OBJECT(TYPE, GIT_TYPE, FACTORY)						\
	GIT_TYPE *object;														\
	int res = repo->get##TYPE(*nameArg, &object);							\
	if(res != GIT_SUCCESS) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			syncRequestObject(object)->handle_);

#define SYNC_CREATE_OBJECT(TYPE, GIT_TYPE, FACTORY)							\
	GIT_TYPE *object;														\
	int res = repo->create##TYPE(&object);									\
	if(res != GIT_SUCCESS) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			newObject(object)->handle_);

namespace gitteh {

struct object_request {
	Persistent<Function> callback;
	Repository *repo;
	int error;
	git_oid oid;
	std::string *name;
	std::string *target;
	void *object;
	bool create;
};

struct exists_request {
	Persistent<Function> callback;
	Repository *repo;
	git_oid oid;
	bool exists;
};

struct open_repo_request {
	Persistent<Function> callback;
	int error;
	String::Utf8Value *path;
	git_repository *repo;
};

struct init_repo_request {
	Persistent<Function> callback;
	int error;
	String::Utf8Value *path;
	bool bare;
	git_repository *repo;
};

struct reflist_request {
	Persistent<Function> callback;
	Repository *repo;
	int error;
	int flags;
	git_strarray refList;
};

Persistent<FunctionTemplate> Repository::constructor_template;

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Repository"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getCommit", GetCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getTag", GetTag);
	NODE_SET_PROTOTYPE_METHOD(t, "getRawObject", GetRawObject);
	NODE_SET_PROTOTYPE_METHOD(t, "getReference", GetReference);

	NODE_SET_PROTOTYPE_METHOD(t, "createWalker", CreateWalker);
	NODE_SET_PROTOTYPE_METHOD(t, "createRawObject", CreateRawObject);
	NODE_SET_PROTOTYPE_METHOD(t, "createTag", CreateTag);
	NODE_SET_PROTOTYPE_METHOD(t, "createTree", CreateTree);
	NODE_SET_PROTOTYPE_METHOD(t, "createCommit", CreateCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "createOidReference", CreateOidRef);
	NODE_SET_PROTOTYPE_METHOD(t, "createSymbolicReference", CreateSymbolicRef);

	NODE_SET_PROTOTYPE_METHOD(t, "listReferences", ListReferences);
	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);
	NODE_SET_PROTOTYPE_METHOD(t, "getIndex", GetIndex);

	NODE_SET_METHOD(target, "openRepository", OpenRepository);
	NODE_SET_METHOD(target, "initRepository", InitRepository);
}

Handle<Value> Repository::OpenRepository(const Arguments& args) {
	HandleScope scope;
	REQ_ARGS(1);
	REQ_STR_ARG(0, pathArg);

	if(HAS_CALLBACK_ARG) {
		open_repo_request *request = new open_repo_request;
		request->callback = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length()-1]));
		request->path = new String::Utf8Value(args[0]);

		eio_custom(EIO_OpenRepository, EIO_PRI_DEFAULT, EIO_AfterOpenRepository, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_repository* repo;
		int result = git_repository_open(&repo, *pathArg);
		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't open repository.", result);
		}

		Handle<Value> constructorArgs[2] = {
			External::New(repo),
			args[0]
		};

		return scope.Close(Repository::constructor_template->GetFunction()
				->NewInstance(2, constructorArgs));
	}
}

int Repository::EIO_OpenRepository(eio_req *req) {
	GET_REQUEST_DATA(open_repo_request);

	reqData->error = git_repository_open(&reqData->repo, **reqData->path);

	return 0;
}

int Repository::EIO_AfterOpenRepository(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(open_repo_request);

	Handle<Value> callbackArgs[2];
 	if(reqData->error) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't open Repository."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		Handle<Value> constructorArgs[2] = {
			External::New(reqData->repo),
			String::New(**reqData->path)
		};
		callbackArgs[0] = Null();
		callbackArgs[1] = Repository::constructor_template->GetFunction()
						->NewInstance(2, constructorArgs);
	}

 	TRIGGER_CALLBACK();

    reqData->callback.Dispose();
 	delete reqData->path;
 	delete reqData;
 	ev_unref(EV_DEFAULT_UC);
	return 0;
}

Handle<Value> Repository::InitRepository(const Arguments& args) {
	HandleScope scope;
	REQ_ARGS(1);
	REQ_STR_ARG(0, pathArg);

	if(HAS_CALLBACK_ARG) {
		init_repo_request *request = new init_repo_request;
		request->callback = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length()-1]));
		request->path = new String::Utf8Value(args[0]);

		if(args.Length() > 2) {
			request->bare = args[1]->BooleanValue();
		}

		eio_custom(EIO_InitRepository, EIO_PRI_DEFAULT, EIO_AfterInitRepository, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		bool bare = false;
		if(args.Length() > 1) {
			bare = args[1]->BooleanValue();
		}

		git_repository* repo;
		int result = git_repository_init(&repo, *pathArg, bare);
		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't init repository.", result);
		}

		Handle<Value> constructorArgs[2] = {
			External::New(repo),
			args[0]
		};

		return scope.Close(Repository::constructor_template->GetFunction()
				->NewInstance(2, constructorArgs));
	}
}

int Repository::EIO_InitRepository(eio_req *req) {
	GET_REQUEST_DATA(init_repo_request);

	reqData->error = git_repository_init(&reqData->repo, **reqData->path, reqData->bare);

	return 0;
}

int Repository::EIO_AfterInitRepository(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(init_repo_request);

	Handle<Value> callbackArgs[2];
	if(reqData->error) {
		Handle<Value> error = CreateGitError(String::New("Couldn't initialize new Repository."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		Handle<Value> constructorArgs[2] = {
			External::New(reqData->repo),
			String::New(**reqData->path)
		};
		callbackArgs[0] = Null();
		callbackArgs[1] = Repository::constructor_template->GetFunction()
						->NewInstance(2, constructorArgs);
	}

	TRIGGER_CALLBACK();

	reqData->callback.Dispose();
	delete reqData->path;
	delete reqData;
	ev_unref(EV_DEFAULT_UC);
	return 0;
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(2);
	REQ_EXT_ARG(0, repoArg);
	REQ_STR_ARG(1, pathArg);

	Repository *repo = new Repository();
	repo->Wrap(args.This());

	repo->repo_ = static_cast<git_repository*>(repoArg->Value());
	repo->path_ = *pathArg;
	repo->odb_ = git_repository_database(repo->repo_);

	args.This()->Set(String::New("path"), String::New(repo->path_), (PropertyAttribute)(ReadOnly | DontDelete));

	// HUGE FUCKING TODO:
	// IN MOTHER FUCKING CAPITALS.
	// FOR SOME FUCKING REASON THE REPOSITORY IS GETTING GARBAGE COLLECTED MID
	// RUN WHEN I DO STRESS TESTS LIKE TRAVERSING COMMIT HISTORY 1000 TIMES
	// SIMULTANEOUSLY. THIS IS FUCKING STUPID BECAUSE THE FUCKING REPO OBJECT
	// STILL HAS A FUCKING REF IN THE FUCKING USERLAND SCRIPT, BUT CUNTS WILL BE
	// FUCKING CUNTS RIGHT? SO ANYWAY I FOUND THIS OUT AFTER LIKE 8 HOURS OF
	// DEBUGGING BULLSHIT FUCKING RANDOM SEGFAULTS ALL OVER THE PLACE. YEAH.
	// AWESOME. I GOT LUCKY IN ONE OF THE SEGFAULTS AND SAW IN GDB THAT THE CUNT
	// WAS ALL FUCKED UP. ANYWAY, I DISCOVERED THAT IF I ATTACHED THE REPO TO
	// THE GLOBAL PROCESS OBJECT THEN REPO WOULDN'T GET REAPO'D AND AS SUCH MY
	// HARD WORK WOULDN'T COME UNDONE LIKE A HOOKER'S BRA CLASP. THERE MUST BE
	// SOME SORT OF WEIRD V8 OPTIMIZATION THAT LOOKS AT LOCAL VARIABLES IN A
	// SCRIPT AND SENSES IF THE CUNTS AREN'T BEING REFERENCED ANYMORE, THEN IT
	// MUST CONSIDER THE LOCAL JS VARIABLE "WEAK" OR SOME SHIT. ANYWAY, IT'S
	// FUCKED AND I DON'T LIKE IT. FOR NOW I'M ADDING REF() HERE TO MAKE SURE
	// THAT A REPO OBJECT CAN NEVER BE MOTHERFUCKING GARBAGE COLLECTED. WHEN I
	// KNOW MORE ABOUT NOT BEING A SHITHEAD AT CODING SHIT THEN I'LL REVISIT
	// THIS SUCKER.
	repo->Ref();
	return args.This();
}

Handle<Value> Repository::CreateCommit(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(HAS_CALLBACK_ARG) {
		ASYNC_PREPARE_CREATE_OBJECT(Commit);
	}
	else {
		SYNC_CREATE_OBJECT(Commit, git_commit, commitFactory_);
	}
}

Handle<Value> Repository::GetCommit(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		ASYNC_PREPARE_GET_OID_OBJECT(Commit, git_commit);
	}
	else {
		SYNC_GET_OID_OBJECT(Commit, git_commit, commitFactory_);
	}
}

Handle<Value> Repository::CreateTree(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(args.Length() > 0) {
		ASYNC_PREPARE_CREATE_OBJECT(Tree);
	}
	else {
		SYNC_CREATE_OBJECT(Tree, git_tree, treeFactory_);
	}
}

Handle<Value> Repository::GetTree(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(args.Length() == 2) {
		ASYNC_PREPARE_GET_OID_OBJECT(Tree, git_tree);
	}
	else {
		SYNC_GET_OID_OBJECT(Tree, git_tree, treeFactory_);
	}
}

Handle<Value> Repository::CreateTag(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(args.Length() > 0) {
		ASYNC_PREPARE_CREATE_OBJECT(Tag);
	}
	else {
		SYNC_CREATE_OBJECT(Tag, git_tag, tagFactory_);
	}
}

Handle<Value> Repository::GetTag(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(args.Length() == 2) {
		ASYNC_PREPARE_GET_OID_OBJECT(Tag, git_tag);
	}
	else {
		SYNC_GET_OID_OBJECT(Tag, git_tag, tagFactory_);
	}
}

Handle<Value> Repository::GetRawObject(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(args.Length() == 2) {
		ASYNC_PREPARE_GET_OID_OBJECT(RawObject, git_rawobj);
	}
	else {
		SYNC_GET_OID_OBJECT(RawObject, git_rawobj, rawObjFactory_);
	}
}

Handle<Value> Repository::CreateRawObject(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(args.Length() > 0) {
		ASYNC_PREPARE_CREATE_OBJECT(RawObject);
	}
	else {
		SYNC_CREATE_OBJECT(RawObject, git_rawobj, rawObjFactory_);
	}
}

Handle<Value> Repository::CreateWalker(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(args.Length() > 0) {
		ASYNC_PREPARE_CREATE_OBJECT(RevWalker);
	}
	else {
		git_revwalk *walker;
		int res = repo->createRevWalker(&walker);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't create revision walker", res);
		}

		RevWalker *walkerObject = repo->wrapRevWalker(walker);
		return scope.Close(walkerObject->handle_);
	}
}

Handle<Value> Repository::GetIndex(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(HAS_CALLBACK_ARG) {
		THROW_ERROR("Unimplemented.");
	}
	else {
		if(repo->index_ == NULL) {
			git_index *index;
			int result = repo->getIndex(&index);

			if(result != GIT_SUCCESS) {
				THROW_GIT_ERROR("Couldn't load index file.", result);
			}

			Handle<Value> arg = External::New(index);
			Handle<Object> instance = Index::constructor_template->GetFunction()->NewInstance(1, &arg);
			repo->index_ = ObjectWrap::Unwrap<Index>(instance);
		}

		return scope.Close(repo->index_->handle_);
	}
}

Handle<Value> Repository::GetReference(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_STR_ARG(0, nameArg);

	if(args.Length() > 1) {
		ASYNC_PREPARE_GET_NAMED_OBJECT(Reference, git_reference);
	}
	else {
		SYNC_GET_NAMED_OBJECT(Reference, git_reference, referenceFactory_);
	}
}

Handle<Value> Repository::CreateSymbolicRef(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(2);
	REQ_STR_ARG(0, nameArg);
	REQ_STR_ARG(1, targetArg);

	if(!nameArg.length()) {
		THROW_ERROR("Please provide a name.");
	}

	if(!targetArg.length()) {
		THROW_ERROR("Please provide a target for the symbolic ref.");
	}

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(object_request);
		request->name = new std::string(*nameArg);
		request->target = new std::string(*targetArg);
		REQUEST_DETACH(repo, EIO_CreateSymbolicRef, EIO_ReturnReference);
	}
	else {
		git_reference *ref;
		int res = git_reference_create_symbolic(&ref, repo->repo_, *nameArg, *targetArg);

		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't create reference.", res);
		}

		return repo->referenceFactory_->syncRequestObject(ref)->handle_;
	}
}

Handle<Value> Repository::CreateOidRef(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(2);
	REQ_STR_ARG(0, nameArg);
	REQ_OID_ARG(1, oidArg);

	if(!nameArg.length()) {
		THROW_ERROR("Please provide a name.");
	}

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(object_request);
		request->name = new std::string(*nameArg);
		char oidStr[40];
		git_oid_fmt(oidStr, &oidArg);
		request->target = new std::string(oidStr, 40);
		REQUEST_DETACH(repo, EIO_CreateOidRef, EIO_ReturnReference);
	}
	else {
		git_reference *ref;
		int res = git_reference_create_oid(&ref, repo->repo_, *nameArg, &oidArg);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't create reference.", res);
		}

		return repo->referenceFactory_->syncRequestObject(ref)->handle_;
	}
}

Handle<Value> Repository::ListReferences(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(reflist_request);

		if(args.Length() > 1) {
			request->flags = args[0]->Int32Value();
		}

		REQUEST_DETACH(repo, EIO_GetRefList, EIO_AfterGetRefList);
	}
	else {
		int flags = 0;
		if(args.Length() > 0) {
			flags = args[0]->Int32Value();
		}

		git_strarray references;
		repo->lockRepository();
		int result = git_reference_listall(&references, repo->repo_, flags);
		repo->unlockRepository();

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't get ref list.", result);
		}

		Handle<Array> refArray = Array::New(references.count);

		for(int i = 0, len = references.count; i < len; i++) {
			refArray->Set(i, String::New(references.strings[i]));
		}

		git_strarray_free(&references);

		return scope.Close(refArray);
	}
}

int Repository::EIO_GetRefList(eio_req *req) {
	GET_REQUEST_DATA(reflist_request);

	reqData->repo->lockRepository();
	reqData->error = git_reference_listall(&reqData->refList, reqData->repo->repo_,
			reqData->flags);
	reqData->repo->unlockRepository();

	return 0;
}

int Repository::EIO_AfterGetRefList(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(reflist_request);
	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];

	if(reqData->error != GIT_SUCCESS) {
		Handle<Value> error = CreateGitError(String::New("Couldn't get ref list."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		Handle<Array> refArray = Array::New(reqData->refList.count);

		for(int i = 0, len = reqData->refList.count; i < len; i++) {
			refArray->Set(i, String::New(reqData->refList.strings[i]));
		}

		callbackArgs[0] = Undefined();
		callbackArgs[1] = refArray;

		git_strarray_free(&reqData->refList);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> Repository::Exists(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(exists_request);
		memcpy(&request->oid, &oidArg, sizeof(git_oid));
		REQUEST_DETACH(repo, EIO_Exists, EIO_AfterExists);
	}
	else {
		return Boolean::New(git_odb_exists(repo->odb_, &oidArg));
	}
}

int Repository::EIO_Exists(eio_req *req) {
	GET_REQUEST_DATA(exists_request);

	reqData->repo->lockRepository();
	reqData->exists = git_odb_exists(reqData->repo->odb_, &reqData->oid);
	reqData->repo->unlockRepository();

	return 0;
}

int Repository::EIO_AfterExists(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(exists_request);
	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();
	Handle<Value> callbackArgs[2];
	callbackArgs[0] = Null();
	callbackArgs[1] = Boolean::New(reqData->exists);
	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;
	return 0;
}

// Boilerplate code can SMFD.
// ==========
// COMMIT EIO
// ==========
FN_ASYNC_GET_OID_OBJECT(Commit, git_commit)
FN_ASYNC_CREATE_OBJECT(Commit, git_commit)
FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(Commit, git_commit, commitFactory_)

// ========
// TREE EIO
// ========
FN_ASYNC_GET_OID_OBJECT(Tree, git_tree)
FN_ASYNC_CREATE_OBJECT(Tree, git_tree)
FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(Tree, git_tree, treeFactory_)

// =======
// TAG EIO
// =======
FN_ASYNC_GET_OID_OBJECT(Tag, git_tag)
FN_ASYNC_CREATE_OBJECT(Tag, git_tag)
FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(Tag, git_tag, tagFactory_)

// =============
// RAWOBJECT EIO
// =============
FN_ASYNC_GET_OID_OBJECT(RawObject, git_rawobj)
FN_ASYNC_CREATE_OBJECT(RawObject, git_rawobj)
FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(RawObject, git_rawobj, rawObjFactory_)

// =======
// REF EIO
// =======
FN_ASYNC_GET_NAMED_OBJECT(Reference, git_reference)
FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(Reference, git_reference, referenceFactory_)

int Repository::EIO_CreateSymbolicRef(eio_req *req) {
	GET_REQUEST_DATA(object_request);

	git_reference *obj;

	reqData->repo->lockRepository();
	reqData->error = git_reference_create_symbolic(&obj, reqData->repo->repo_,
			reqData->name->c_str(), reqData->target->c_str());
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		reqData->object = obj;
	}

	delete reqData->name;
	delete reqData->target;

	return 0;
}

int Repository::EIO_CreateOidRef(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	reqData->repo->lockRepository();

	// Ignoring the result of this, as we know it's definitely a good oid.
	git_oid oid;
	git_oid_mkstr(&oid, reqData->target->c_str());

	git_reference *obj;
	reqData->error = git_reference_create_oid(&obj, reqData->repo->repo_,
			reqData->name->c_str(), &oid);

	reqData->repo->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		reqData->object = obj;
	}

	delete reqData->name;
	delete reqData->target;

	return 0;
}

// ===========
// REVWALK EIO
// ===========
FN_ASYNC_CREATE_OBJECT(RevWalker, git_revwalk)
FN_ASYNC_RETURN_OBJECT_VIA_WRAP(RevWalker, git_revwalk)

Repository::Repository() {
	CREATE_MUTEX(gitLock_);

	commitFactory_ = new ObjectFactory<Repository, Commit, git_commit>(this);
	referenceFactory_ = new ObjectFactory<Repository, Reference, git_reference>(this);
	treeFactory_ = new ObjectFactory<Repository, Tree, git_tree>(this);
	tagFactory_ = new ObjectFactory<Repository, Tag, git_tag>(this);
	rawObjFactory_ = new ObjectFactory<Repository, RawObject, git_rawobj>(this);
}

Repository::~Repository() {
	delete commitFactory_;
	delete referenceFactory_;
	delete treeFactory_;
	delete tagFactory_;
	delete rawObjFactory_;

	close();
}

void Repository::close() {
	if(repo_) {
		git_repository_free(repo_);
		repo_ = NULL;
	}
}

int Repository::getCommit(git_oid *id, git_commit **commit) {
	LOCK_MUTEX(gitLock_);
	int result;
	result = git_commit_lookup(commit, repo_, id);
	UNLOCK_MUTEX(gitLock_);
	return result;
}

int Repository::createCommit(git_commit **commit) {
	int result;
	
	LOCK_MUTEX(gitLock_);
	result = git_commit_new(commit, repo_);
	UNLOCK_MUTEX(gitLock_);

	return result;
}

int Repository::createTree(git_tree **tree) {
	int result;
	
	LOCK_MUTEX(gitLock_);
	result = git_tree_new(tree, repo_);
	UNLOCK_MUTEX(gitLock_);
	
	return result;
}

int Repository::getTree(git_oid *id, git_tree **tree) {
	int result;
	
	LOCK_MUTEX(gitLock_);
	result = git_tree_lookup(tree, repo_, id);
	UNLOCK_MUTEX(gitLock_);
	
	return result;
}

int Repository::createTag(git_tag **tag) {
	int result;

	LOCK_MUTEX(gitLock_);
	result = git_tag_new(tag, repo_);
	UNLOCK_MUTEX(gitLock_);

	return result;
}

int Repository::getTag(git_oid *id, git_tag **tag) {
	int result;

	LOCK_MUTEX(gitLock_);
	result = git_tag_lookup(tag, repo_, id);
	UNLOCK_MUTEX(gitLock_);

	return result;
}

int Repository::getReference(const char* name, git_reference** ref) {
	int result;

	LOCK_MUTEX(gitLock_);
	result = git_reference_lookup(ref, repo_, name);
	UNLOCK_MUTEX(gitLock_);

	return result;
}

int Repository::getRawObject(git_oid *id, git_rawobj **objPtr) {
	int result;

	LOCK_MUTEX(gitLock_);
	git_rawobj *obj = new git_rawobj;
	result = git_odb_read(obj, odb_, id);
	UNLOCK_MUTEX(gitLock_);
	
	if(result == GIT_SUCCESS) {
		*objPtr = obj;
	}

	return result;
}

int Repository::createRevWalker(git_revwalk **walker) {
	int result;

	LOCK_MUTEX(gitLock_);
	result = git_revwalk_new(walker, repo_);
	UNLOCK_MUTEX(gitLock_);

	return result;
}

RevWalker *Repository::wrapRevWalker(git_revwalk *walker) {
	HandleScope scope;

	Handle<Value> constructorArgs[1] = { External::New(walker) };
	Handle<Object> jsObject = RevWalker::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

	RevWalker *walkerObj = ObjectWrap::Unwrap<RevWalker>(jsObject);
	walkerObj->repo_ = this;

	return walkerObj;
}

int Repository::createRawObject(git_rawobj** rawObj) {
	*rawObj = new git_rawobj;
	(*rawObj)->len = 0;
	(*rawObj)->type = GIT_OBJ_BAD;

	return GIT_SUCCESS;
}

int Repository::getIndex(git_index **index) {
	lockRepository();
	int result = git_repository_index(index, repo_);
	unlockRepository();
	if(result == GIT_EBAREINDEX) {
		lockRepository();
		result = git_index_open_bare(index, path_);
		unlockRepository();
	}

	return result;
}

void Repository::lockRepository() {
	LOCK_MUTEX(gitLock_);
}

void Repository::unlockRepository() {
	UNLOCK_MUTEX(gitLock_);
}

} // namespace gitteh
