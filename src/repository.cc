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
/*#include "commit.h"
#include "tree.h"
#include "index.h"
#include "tag.h"
#include "rev_walker.h"
#include "ref.h"
#include "blob.h"*/
#include <sys/time.h>

// DANGER, WILL ROBINSON!
// The nastiest code that will ever rape your eyeballs follows.
// While writing async callbacks for getting/creating all the different objects
// I decided that no, I didn't want to copy and paste the same blocks of code
// 6 trillion times. Instead, I write a bunch of ugly motherfucking macros that
// I have no hope of maintaining in future. Fuck I'm rad.

#define GET_BATON(REQUESTTYPE)												\
	REQUESTTYPE *baton = static_cast<REQUESTTYPE*>(req->data);

#define REQUEST_CLEANUP()													\
    reqData->callback.Dispose();											\
 	ev_unref(EV_DEFAULT_UC);												\
 	reqData->repo->Unref();													\
 	delete reqData;															\
	return 0;

#define REQUEST_DETACH(OBJ, FN, AFTERFN)									\
	OBJ->Ref();																\
	uv_queue_work(uv_default_loop(), &baton->req, FN, AFTERFN);				\
	ev_ref(EV_DEFAULT_UC);													\
	return scope.Close(Undefined());

#define CREATE_ASYNC_REQUEST(BATONCLASS)									\
	BATONCLASS *baton = new BATONCLASS();									\
	baton->callback = Persistent<Function>::New(callbackArg);				\
	baton->req.data = baton;												\
	baton->repo = repo;

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
	void Repository::EIO_Get##TYPE(eio_req *req) {							\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE *object;													\
		reqData->error = reqData->repo->get##TYPE(&reqData->oid,			\
				&object);													\
		if(reqData->error == GIT_OK) {									\
			reqData->object = object;										\
		}																	\
	}

#define FN_ASYNC_GET_NAMED_OBJECT(TYPE, GIT_TYPE)							\
	void Repository::EIO_Get##TYPE(eio_req *req) {							\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE *object;													\
		reqData->error = reqData->repo->get##TYPE(reqData->name->c_str(),	\
				&object);													\
		if(reqData->error == GIT_OK) {									\
			reqData->object = object;										\
		}																	\
		delete reqData->name;												\
	}

#define FN_ASYNC_RETURN_OBJECT_VIA_FACTORY(TYPE, GIT_TYPE, FACTORY)			\
	int Repository::EIO_Return##TYPE(eio_req *req) {						\
		HandleScope scope;													\
		GET_REQUEST_DATA(object_request);									\
		ev_unref(EV_DEFAULT_UC);											\
		reqData->repo->Unref();												\
		Handle<Value> callbackArgs[2];										\
		if(reqData->error != GIT_OK) {									\
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
		if(reqData->error != GIT_OK) {									\
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
	void Repository::EIO_Create##TYPE(eio_req *req) {						\
		GET_REQUEST_DATA(object_request);									\
		GIT_TYPE* object;													\
		reqData->error = reqData->repo->create##TYPE(&object);				\
		if(reqData->error == GIT_OK) {									\
			reqData->object = object;										\
		}																	\
	}

#define ASYNC_PREPARE_GET_OID_OBJECT(TYPE, GIT_TYPE)						\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	memcpy(&request->oid, &oidArg, sizeof(git_oid));						\
	request->create = false;												\
	REQUEST_DETACH(repo, EIO_Get##TYPE, EIO_Return##TYPE);

#define ASYNC_PREPARE_GET_NAMED_OBJECT(TYPE, GIT_TYPE)						\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	request->name = new std::string(*nameArg);								\
	request->create = false;												\
	REQUEST_DETACH(repo, EIO_Get##TYPE, EIO_Return##TYPE);

#define ASYNC_PREPARE_CREATE_OBJECT(TYPE)									\
	REQ_FUN_ARG(args.Length() - 1, callbackArg);							\
	CREATE_ASYNC_REQUEST(object_request);									\
	request->create = true;													\
	REQUEST_DETACH(repo, EIO_Create##TYPE, EIO_Return##TYPE);

#define SYNC_GET_OID_OBJECT(TYPE, GIT_TYPE, FACTORY)						\
	GIT_TYPE *object;														\
	int res = repo->get##TYPE(&oidArg, &object);							\
	if(res != GIT_OK) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			syncRequestObject(object)->handle_);

#define SYNC_GET_NAMED_OBJECT(TYPE, GIT_TYPE, FACTORY)						\
	GIT_TYPE *object;														\
	int res = repo->get##TYPE(*nameArg, &object);							\
	if(res != GIT_OK) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			syncRequestObject(object)->handle_);

#define SYNC_CREATE_OBJECT(TYPE, GIT_TYPE, FACTORY)							\
	GIT_TYPE *object;														\
	int res = repo->create##TYPE(&object);									\
	if(res != GIT_OK) {												\
		THROW_GIT_ERROR("Git error.", res);									\
	}																		\
	return scope.Close(repo->FACTORY->										\
			newObject(object)->handle_);

namespace gitteh {
static Persistent<String> repo_class_symbol;
static Persistent<String> path_symbol;

static Persistent<String> git_dir_symbol;
static Persistent<String> object_dir_symbol;
static Persistent<String> index_file_symbol;
static Persistent<String> work_tree_symbol;

struct OpenRepoBaton {
	uv_work_t req;
	Persistent<Function> callback;
	int error;
	std::string path;
	git_repository *repo;
};

struct ExistsBaton {
	uv_work_t req;
	Persistent<Function> callback;
	Repository *repo;
	git_oid oid;
	bool exists;
};

/*
struct object_request {
	Persistent<Function> callback;
	Repository *repo;
	int error;
	git_oid oid;
	std::string *name;
	std::string *target;
	void *object;
	void *wrappedObject;
	bool create;
};

struct open_repo2_request {
	Persistent<Function> callback;
	int error;
	std::string *gitDir;
	std::string *objectDir;
	std::string *indexFile;
	std::string *workTree;
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

struct ref_pack_request {
	Persistent<Function> callback;
	Repository *repo;
	int error;
};

struct index_request {
	Persistent<Function> callback;
	Repository *repo;
};*/

Persistent<FunctionTemplate> Repository::constructor_template;

Repository::Repository() {
	/*CREATE_MUTEX(gitLock_);
	CREATE_MUTEX(refLock_);

	commitCache_ = new WrappedGitObjectCache<Commit, git_commit>(this);
	referenceCache_ = new WrappedGitObjectCache<Reference, git_reference>(this);
	treeCache_ = new WrappedGitObjectCache<Tree, git_tree>(this);
	tagCache_ = new WrappedGitObjectCache<Tag, git_tag>(this);
	blobCache_ = new WrappedGitObjectCache<Blob, git_blob>(this);

	index_ = NULL;*/

	odb_ = NULL;
	repo_ = NULL;
}

Repository::~Repository() {
	if(odb_) {
		git_odb_free(odb_);
		odb_ = NULL;
	}

	if(repo_) {
		git_repository_free(repo_);
		repo_ = NULL;
	}

	/*delete commitCache_;
	delete referenceCache_;
	delete treeCache_;
	delete tagCache_;
	delete blobCache_;

	close();*/
}

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	repo_class_symbol = NODE_PSYMBOL("Repository");
	path_symbol = NODE_PSYMBOL("path");
	git_dir_symbol = NODE_PSYMBOL("gitDirectory");
	object_dir_symbol = NODE_PSYMBOL("objectDirectory");
	index_file_symbol = NODE_PSYMBOL("indexFile");
	work_tree_symbol = NODE_PSYMBOL("workTree");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(repo_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);
/*
	NODE_SET_PROTOTYPE_METHOD(t, "getCommit", GetCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getTag", GetTag);
	NODE_SET_PROTOTYPE_METHOD(t, "getReference", GetReference);
	NODE_SET_PROTOTYPE_METHOD(t, "getBlob", GetBlob);

	NODE_SET_PROTOTYPE_METHOD(t, "createWalker", CreateWalker);
	NODE_SET_PROTOTYPE_METHOD(t, "createTag", CreateTag);
	NODE_SET_PROTOTYPE_METHOD(t, "createTree", CreateTree);
	NODE_SET_PROTOTYPE_METHOD(t, "createBlob", CreateBlob);
	NODE_SET_PROTOTYPE_METHOD(t, "createCommit", CreateCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "createOidReference", CreateOidRef);
	NODE_SET_PROTOTYPE_METHOD(t, "createSymbolicReference", CreateSymbolicRef);

	NODE_SET_PROTOTYPE_METHOD(t, "listReferences", ListReferences);
	NODE_SET_PROTOTYPE_METHOD(t, "packReferences", PackReferences);
	*/
	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);
	/*
	NODE_SET_PROTOTYPE_METHOD(t, "getIndex", GetIndex);
*/
	NODE_SET_METHOD(target, "openRepository", OpenRepository);
	// NODE_SET_METHOD(target, "initRepository", InitRepository);

	// target->Set(repo_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, repoArg);

	git_repository *repo = static_cast<git_repository*>(repoArg->Value());
	const char *repoPath = git_repository_path(repo);
	git_odb *odb;
	if(git_repository_odb(&odb, repo) != GIT_OK) {
		return scope.Close(ThrowGitError());
	}

	Repository *repoObj = new Repository();
	repoObj->Wrap(args.This());

	repoObj->repo_ = repo;
	repoObj->path_ = repoPath;
	repoObj->odb_ = odb;

	args.This()->Set(path_symbol, String::New(repoObj->path_),
			(PropertyAttribute)(ReadOnly | DontDelete));

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
	repoObj->Ref();
	return args.This();
}

Handle<Value> Repository::OpenRepository(const Arguments& args) {
	HandleScope scope;
	REQ_ARGS(1);
/*
	if(args[0]->IsObject()) {
		Handle<Object> pathsObj = Handle<Object>::Cast(args[0]);
		if(pathsObj->Get(git_dir_symbol)->Equals(Null())) {
			THROW_ERROR("Git directory is required.");
		}

		String::Utf8Value gitDir(pathsObj->Get(git_dir_symbol));
		String::Utf8Value objDir(pathsObj->Get(object_dir_symbol));
		String::Utf8Value indexFile(pathsObj->Get(index_file_symbol));
		String::Utf8Value workTree(pathsObj->Get(work_tree_symbol));

		if(HAS_CALLBACK_ARG) {
			open_repo2_request *request = new open_repo2_request;
			request->callback = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length()-1]));
			request->gitDir = new std::string(*gitDir);
			if(!pathsObj->Get(object_dir_symbol)->IsUndefined()) {
				request->objectDir = new std::string(*objDir);
			}
			else {
				request->objectDir = NULL;
			}
			if(!pathsObj->Get(index_file_symbol)->IsUndefined()) {
				request->indexFile = new std::string(*indexFile);
			}
			else {
				request->indexFile = NULL;
			}
			if(!pathsObj->Get(work_tree_symbol)->IsUndefined()) {
				request->workTree = new std::string(*workTree);
			}
			else {
				request->workTree = NULL;
			}

			eio_custom(EIO_OpenRepository2, EIO_PRI_DEFAULT, EIO_AfterOpenRepository2, request);
			ev_ref(EV_DEFAULT_UC);

			return Undefined();
		}
		else {
			git_repository* repo;

			const char *_gitDir = *gitDir;
			const char *_objDir = NULL;
			if(!pathsObj->Get(object_dir_symbol)->IsUndefined()) {
				_objDir = *objDir;
			}
			const char *_indexFile = NULL;
			if(!pathsObj->Get(index_file_symbol)->IsUndefined()) {
				_indexFile = *indexFile;
			}
			const char *_workTree = NULL;
			if(!pathsObj->Get(work_tree_symbol)->IsUndefined()) {
				_workTree = *workTree;
			}

			int result = git_repository_open2(&repo, _gitDir, _objDir, _indexFile,
					_workTree);
			if(result != GIT_OK) {
				THROW_GIT_ERROR("Couldn't open repository.", result);
			}

			Handle<Value> constructorArgs[2] = {
				External::New(repo),
				pathsObj->Get(git_dir_symbol)
			};

			return scope.Close(Repository::constructor_template->GetFunction()
					->NewInstance(2, constructorArgs));
		}
	}
	else if(args[0]->IsString()) {*/
		REQ_STR_ARG(0, pathArg);

		if(HAS_CALLBACK_ARG) {
			OpenRepoBaton *baton = new OpenRepoBaton;
			baton->callback = Persistent<Function>::New(Handle<Function>::Cast(args[args.Length()-1]));
			baton->req.data = baton;
			baton->path = std::string(*pathArg);

			uv_queue_work(uv_default_loop(), &baton->req, AsyncOpenRepository,
				AsyncAfterOpenRepository);

			return Undefined();
		}
		else {
			git_repository* repo;
			int result = git_repository_open(&repo, *pathArg);
			if(result != GIT_OK) {
				return scope.Close(ThrowGitError());
				// THROW_GIT_ERROR("Couldn't open repository.", result);
			}

			Handle<Value> constructorArgs[1] = {
				External::New(repo)
			};

			return scope.Close(Repository::constructor_template->GetFunction()
					->NewInstance(1, constructorArgs));
		}
	// }

	THROW_ERROR("Invalid argument.");
}

void Repository::AsyncOpenRepository(uv_work_t *req) {
	GET_BATON(OpenRepoBaton)

	baton->error = git_repository_open(&baton->repo, baton->path.c_str());
}

void Repository::AsyncAfterOpenRepository(uv_work_t *req) {
	HandleScope scope;
	OpenRepoBaton *baton = GetBaton<OpenRepoBaton>(req);

	Handle<Value> callbackArgs[2];
 	if(baton->error) {
 		Handle<Value> error = CreateGitError();
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		Handle<Value> constructorArgs[1] = {
			External::New(baton->repo)
		};
		callbackArgs[0] = Null();
		callbackArgs[1] = Repository::constructor_template->GetFunction()
						->NewInstance(1, constructorArgs);
	}

 	TRIGGER_CALLBACK();

    baton->callback.Dispose();
 	delete baton;
}
/*
void Repository::EIO_OpenRepository2(eio_req *req) {
	GET_REQUEST_DATA(open_repo2_request);

	const char *_gitDir = reqData->gitDir->c_str();
	const char *_objDir = NULL;
	if(reqData->objectDir) {
		_objDir = reqData->objectDir->c_str();
	}
	const char *_indexFile = NULL;
	if(reqData->indexFile) {
		_indexFile = reqData->indexFile->c_str();
	}
	const char *_workTree = NULL;
	if(reqData->workTree) {
		_workTree = reqData->workTree->c_str();
	}

	reqData->error = git_repository_open2(&reqData->repo, _gitDir, _objDir,
			_indexFile, _workTree);

	if(reqData->objectDir) {
		delete reqData->objectDir;
	}
	if(reqData->indexFile) {
		delete reqData->indexFile;
	}
	if(reqData->workTree) {
		delete reqData->workTree;
	}
}

int Repository::EIO_AfterOpenRepository2(eio_req *req) {
	HandleScope scope;
		GET_REQUEST_DATA(open_repo2_request);

		Handle<Value> callbackArgs[2];
	 	if(reqData->error) {
	 		Handle<Value> error = CreateGitError(String::New("Couldn't open Repository."), reqData->error);
	 		callbackArgs[0] = error;
	 		callbackArgs[1] = Null();
		}
		else {
			Handle<Value> constructorArgs[2] = {
				External::New(reqData->repo),
				String::New(reqData->gitDir->c_str())
			};
			callbackArgs[0] = Null();
			callbackArgs[1] = Repository::constructor_template->GetFunction()
							->NewInstance(2, constructorArgs);

			delete reqData->gitDir;
		}

	 	TRIGGER_CALLBACK();

	    reqData->callback.Dispose();
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
		if(result != GIT_OK) {
			// THROW_GIT_ERROR("Couldn't init repository.", result);
			return scope.Close(ThrowGitError());
		}

		Handle<Value> constructorArgs[2] = {
			External::New(repo),
			args[0]
		};

		return scope.Close(Repository::constructor_template->GetFunction()
				->NewInstance(2, constructorArgs));
	}
}

void Repository::EIO_InitRepository(eio_req *req) {
	GET_REQUEST_DATA(init_repo_request);

	reqData->error = git_repository_init(&reqData->repo, **reqData->path, reqData->bare);
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

Handle<Value> Repository::CreateCommit(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OBJ_ARG(0, commitObjArg);

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(Commit::SaveObject(commitObjArg, repo, callback, true));
}

Handle<Value> Repository::CreateBlob(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OBJ_ARG(0, blobObjArg);

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(Blob::SaveObject(blobObjArg, repo, callback, true));
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
		git_commit *commit;
		int result = repo->getCommit(&oidArg, &commit);
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't get commit.", result);
		}

		return scope.Close(repo->commitCache_->syncRequest(commit));
	}
}

void Repository::EIO_GetCommit(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	git_commit *commit;
	reqData->error = reqData->repo->getCommit(&reqData->oid,
			&commit);

	if(reqData->error == GIT_OK) {
		reqData->object = commit;

		Commit *commitObject;
		reqData->error = reqData->repo->commitCache_->asyncRequest(
				commit, &commitObject);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = commitObject;
		}
	}
}

int Repository::EIO_ReturnCommit(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(object_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New(
				"Couldn't get commit."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Commit *wrappedCommit = static_cast<Commit*>(reqData->wrappedObject);
		wrappedCommit->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(wrappedCommit->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
}

Handle<Value> Repository::CreateTree(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	THROW_ERROR("Unimplemented.");
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
		git_tree *tree;
		int result = repo->getTree(&oidArg, &tree);
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't get tree.", result);
		}

		return scope.Close(repo->treeCache_->syncRequest(tree));
	}
}

void Repository::EIO_GetTree(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	git_tree *tree;
	reqData->error = reqData->repo->getTree(&reqData->oid,
			&tree);

	if(reqData->error == GIT_OK) {
		reqData->object = tree;

		Tree *treeObject;
		reqData->error = reqData->repo->treeCache_->asyncRequest(
				tree, &treeObject);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = treeObject;
		}
	}
}

int Repository::EIO_ReturnTree(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(object_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New(
				"Couldn't get tree."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Tree *wrappedTree = static_cast<Tree*>(reqData->wrappedObject);
		wrappedTree->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(wrappedTree->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
}

Handle<Value> Repository::CreateTag(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OBJ_ARG(0, tagObjArg);

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(Tag::SaveObject(tagObjArg, repo, callback, true));
}

Handle<Value> Repository::GetTag(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		ASYNC_PREPARE_GET_OID_OBJECT(Tag, git_tag);
	}
	else {
		git_tag *tag;
		int result = repo->getTag(&oidArg, &tag);
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't get tag.", result);
		}

		return scope.Close(repo->tagCache_->syncRequest(tag));
	}
}

void Repository::EIO_GetTag(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	git_tag *tag;
	reqData->error = reqData->repo->getTag(&reqData->oid,
			&tag);

	if(reqData->error == GIT_OK) {
		reqData->object = tag;

		Tag *tagObject;
		reqData->error = reqData->repo->tagCache_->asyncRequest(
				tag, &tagObject);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = tagObject;
		}
	}
}

int Repository::EIO_ReturnTag(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(object_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New(
				"Couldn't get tag."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Tag *wrappedTag = static_cast<Tag*>(reqData->wrappedObject);
		wrappedTag->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(wrappedTag->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
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
		if(res != GIT_OK) {
			THROW_GIT_ERROR("Couldn't create revision walker", res);
		}

		RevWalker *walkerObject = repo->wrapRevWalker(walker);
		return scope.Close(walkerObject->handle_);
	}
}

Handle<Value> Repository::GetIndex(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(repo->index_ == NULL) {
		Handle<Object> instance = Index::constructor_template->GetFunction()->NewInstance(0, NULL);
		repo->index_ = ObjectWrap::Unwrap<Index>(instance);
		repo->index_->setOwner(repo);
	}

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		if(!repo->index_->isInitialized()) {
			CREATE_ASYNC_REQUEST(index_request);
			repo->index_->registerInitInterest();
			REQUEST_DETACH(repo, EIO_InitIndex, EIO_ReturnIndex);
		}
		else {
			Handle<Value> callbackArgs[2];
			callbackArgs[0] = Null();
			callbackArgs[1] = Local<Object>::New(repo->index_->handle_);
			callbackArg->Call(Context::GetCurrent()->Global(), 2, callbackArgs);
		}

		return Undefined();
	}
	else {
		if(!repo->index_->isInitialized()) {
			repo->index_->registerInitInterest();
			repo->index_->waitForInitialization();
			repo->index_->removeInitInterest();
			repo->index_->ensureInitDone();
		}

		if(repo->index_->initError_ != GIT_OK) {
			THROW_GIT_ERROR("Couldn't get index.", repo->index_->initError_);
		}

		return scope.Close(repo->index_->handle_);
	}
}

void Repository::EIO_InitIndex(eio_req *req) {
	GET_REQUEST_DATA(index_request);
	reqData->repo->index_->waitForInitialization();
}

int Repository::EIO_ReturnIndex(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(index_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	reqData->repo->index_->removeInitInterest();
	reqData->repo->index_->ensureInitDone();

	Handle<Value> callbackArgs[2];

	if(reqData->repo->index_->initError_ != GIT_OK) {
		Handle<Value> e = CreateGitError(String::New("Couldn't get ref list."),
				reqData->repo->index_->initError_);
		callbackArgs[0] = e;
		callbackArgs[1] = Null();
	}
	else {
		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(reqData->repo->index_->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
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
		repo->lockRefs();

		git_reference *ref;

		int res = repo->getReference(*nameArg, &ref);
		if(res != GIT_OK) {
			repo->unlockRefs();
			THROW_GIT_ERROR("Git error.", res);
		}

		Handle<Value> refObj = repo->referenceCache_->syncRequest(ref);
		repo->unlockRefs();
		return scope.Close(refObj);
	}
}

void Repository::EIO_GetReference(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	git_reference *ref;

	reqData->repo->lockRefs();
	reqData->error = reqData->repo->getReference(reqData->name->c_str(),
			&ref);

	if(reqData->error == GIT_OK) {
		reqData->object = ref;

		Reference *refObject;
		reqData->error = reqData->repo->referenceCache_->asyncRequest(
				ref, &refObject);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = refObject;
		}
	}
	reqData->repo->unlockRefs();

	delete reqData->name;
}

int Repository::EIO_ReturnReference(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(object_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New(
				"Couldn't get ref."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Reference *wrappedReference = static_cast<Reference*>(reqData->wrappedObject);
		wrappedReference->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(wrappedReference->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
}

Handle<Value> Repository::GetBlob(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(args.Length() == 2) {
		ASYNC_PREPARE_GET_OID_OBJECT(Blob, git_blob);
	}
	else {
		git_blob *blob;
		int result = repo->getBlob(&oidArg, &blob);
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't get blob.", result);
		}

		return scope.Close(repo->blobCache_->syncRequest(blob));
	}
}

void Repository::EIO_GetBlob(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	git_blob *blob;
	reqData->error = reqData->repo->getBlob(&reqData->oid,
			&blob);

	if(reqData->error == GIT_OK) {
		reqData->object = blob;

		Blob *blobObject;
		reqData->error = reqData->repo->blobCache_->asyncRequest(
				blob, &blobObject);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = blobObject;
		}
	}
}

int Repository::EIO_ReturnBlob(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(object_request);

	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New(
				"Couldn't get blob."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Undefined();
	}
	else {
		Blob *wrappedBlob = static_cast<Blob*>(reqData->wrappedObject);
		wrappedBlob->ensureWrapped();

		callbackArgs[0] = Undefined();
		callbackArgs[1] = Local<Object>::New(wrappedBlob->handle_);
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
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
		repo->lockRefs();

		git_reference *ref;
		int res = git_reference_create_symbolic(&ref, repo->repo_, *nameArg, *targetArg);

		if(res != GIT_OK) {
			repo->unlockRefs();
			THROW_GIT_ERROR("Couldn't create reference.", res);
		}

		Handle<Value> refObj = repo->referenceCache_->syncRequest(ref);
		repo->unlockRefs();

		return scope.Close(refObj);
	}
}

void Repository::EIO_CreateSymbolicRef(eio_req *req) {
	GET_REQUEST_DATA(object_request);

	git_reference *ref;

	reqData->repo->lockRepository();
	reqData->error = git_reference_create_symbolic(&ref, reqData->repo->repo_,
			reqData->name->c_str(), reqData->target->c_str());
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_OK) {
		reqData->object = ref;

		Reference *refObj;
		reqData->error = reqData->repo->referenceCache_->asyncRequest(ref,
				&refObj);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = refObj;
		}
	}

	delete reqData->name;
	delete reqData->target;
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
		repo->lockRefs();

		git_reference *ref;
		repo->lockRepository();
		int res = git_reference_create_oid(&ref, repo->repo_, *nameArg, &oidArg);
		repo->unlockRepository();

		if(res != GIT_OK) {
			repo->unlockRefs();
			THROW_GIT_ERROR("Couldn't create reference.", res);
		}

		Handle<Value> refObj = repo->referenceCache_->syncRequest(ref);
		repo->unlockRefs();

		return scope.Close(refObj);
	}
}

void Repository::EIO_CreateOidRef(eio_req *req) {
	GET_REQUEST_DATA(object_request);
	reqData->repo->lockRepository();

	// Ignoring the result of this, as we know it's definitely a good oid.
	git_oid oid;
	git_oid_fromstr(&oid, reqData->target->c_str());

	git_reference *ref;
	reqData->error = git_reference_create_oid(&ref, reqData->repo->repo_,
			reqData->name->c_str(), &oid);

	reqData->repo->unlockRepository();

	if(reqData->error == GIT_OK) {
		reqData->object = ref;

		Reference *refObj;
		reqData->error = reqData->repo->referenceCache_->asyncRequest(ref,
				&refObj);

		if(reqData->error == GIT_OK) {
			reqData->wrappedObject = refObj;
		}
	}

	delete reqData->name;
	delete reqData->target;
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

		if(result != GIT_OK) {
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

void Repository::EIO_GetRefList(eio_req *req) {
	GET_REQUEST_DATA(reflist_request);

	reqData->repo->lockRepository();
	reqData->error = git_reference_listall(&reqData->refList, reqData->repo->repo_,
			reqData->flags);
	reqData->repo->unlockRepository();
}

int Repository::EIO_AfterGetRefList(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(reflist_request);
	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];

	if(reqData->error != GIT_OK) {
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

int Repository::DoRefPacking() {
	// Fun times abound!
	int result;

	// BIG TODO: probably not very good chance of it happening. But what should
	// we do if the ref ptr update process fails anywhere? For now I'm just ignoring
	// the return status and pretending nothing could go wrong, but it might.

	lockRefs();

	// First step, grab all the cached reference from object factory.
	int refCount;
	Reference **refList = referenceCache_->getAllObjects(&refCount);

	// Now we go through them all, lock them, and save what their current name is.
	// We'll need the names for lata.
	std::string **refNames = new std::string*[refCount];
	for(int i = 0; i < refCount; i++) {
		refList[i]->lock();

		if(refList[i]->deleted_) {
			refList[i]->unlock();
			refNames[i] = NULL;
			continue;
		}

		refNames[i] = new std::string(git_reference_name(refList[i]->ref_));
	}

	// Okay. Time to make shit happen.
	lockRepository();
	result = git_reference_packall(repo_);
	unlockRepository();

	if(result == GIT_OK) {
		// Mo'fuckin' success!!!! Now we update all the existing references with
		// their ... new reference pointer to their ... reference. Awesome.
		for(int i = 0; i < refCount; i++) {
			if(refNames[i] != NULL) {
				git_reference *newRef;
				git_reference *oldRef = refList[i]->ref_;

				git_reference_lookup(&newRef, repo_, refNames[i]->c_str());
				refList[i]->ref_ = newRef;

				referenceCache_->updateCacheRef(oldRef, newRef);

				// It's now safe to unlock this ref, as it's valid once again.
				refList[i]->unlock();
			}
		}
	}

	delete [] refList;
	for(int i = 0; i < refCount; i++) {
		if(refNames[i] != NULL) {
			delete refNames[i];
		}
	}
	delete [] refNames;

	unlockRefs();

	return result;
}

Handle<Value> Repository::PackReferences(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(ref_pack_request);

		REQUEST_DETACH(repo, EIO_PackRefs, EIO_AfterPackRefs);
	}
	else {
		int result = repo->DoRefPacking();
		if(result != GIT_OK) {
			THROW_GIT_ERROR("Couldn't pack refs.", result);
		}

		return True();
	}
}

void Repository::EIO_PackRefs(eio_req *req) {
	GET_REQUEST_DATA(ref_pack_request);

	reqData->error = reqData->repo->DoRefPacking();
}

int Repository::EIO_AfterPackRefs(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA(ref_pack_request);
	ev_unref(EV_DEFAULT_UC);
	reqData->repo->Unref();

	Handle<Value> callbackArgs[2];

	if(reqData->error != GIT_OK) {
		Handle<Value> error = CreateGitError(String::New("Couldn't pack refs."), reqData->error);
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		callbackArgs[0] = Undefined();
		callbackArgs[1] = True();
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}*/

Handle<Value> Repository::Exists(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oidArg);

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		CREATE_ASYNC_REQUEST(ExistsBaton);
		memcpy(&baton->oid, &oidArg, sizeof(git_oid));
		REQUEST_DETACH(repo, AsyncExists, AsyncAfterExists);
	}
	else {
		return Boolean::New(git_odb_exists(repo->odb_, &oidArg));
	}
}

void Repository::AsyncExists(uv_work_t *req) {
	GET_BATON(ExistsBaton)

	baton->exists = git_odb_exists(baton->repo->odb_, &baton->oid);
}

void Repository::AsyncAfterExists(uv_work_t *req) {
	HandleScope scope;
	GET_BATON(ExistsBaton)

	baton->repo->Unref();
	Handle<Value> callbackArgs[2];
	callbackArgs[0] = Null();
	callbackArgs[1] = Boolean::New(baton->exists);
	TRIGGER_CALLBACK();
	baton->callback.Dispose();
	delete baton;
}

/*
// ===========
// REVWALK EIO
// ===========
FN_ASYNC_CREATE_OBJECT(RevWalker, git_revwalk)
FN_ASYNC_RETURN_OBJECT_VIA_WRAP(RevWalker, git_revwalk)

void Repository::close() {
	
}

int Repository::getCommit(git_oid *id, git_commit **commit) {
	LOCK_MUTEX(gitLock_);
	int result;
	result = git_commit_lookup(commit, repo_, id);
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

int Repository::getBlob(git_oid *id, git_blob **blob) {
	int result;

	LOCK_MUTEX(gitLock_);
	result = git_blob_lookup(blob, repo_, id);
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

void Repository::notifyIndexDead() {
	index_ = NULL;
}

void Repository::lockRepository() {
	LOCK_MUTEX(gitLock_);
}

void Repository::unlockRepository() {
	UNLOCK_MUTEX(gitLock_);
}

void Repository::lockRefs() {
	LOCK_MUTEX(refLock_);
}

void Repository::unlockRefs() {
	UNLOCK_MUTEX(refLock_);
}*/

} // namespace gitteh
