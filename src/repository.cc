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

#define GET_REQUEST_DATA()													\
	object_get_request *requestData =										\
		static_cast<object_get_request*>(req->data);

#define REQUEST_CLEANUP()													\
    requestData->callback.Dispose();										\
 	ev_unref(EV_DEFAULT_UC);												\
 	requestData->repo->Unref();												\
	return 0;

namespace gitteh {

struct object_get_request {
	Persistent<Function> callback;
	Repository *repo;
	void *object;
	git_oid oid;
	int error;
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

	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);

	t->InstanceTemplate()->SetAccessor(String::New("index"), IndexGetter);

	target->Set(String::New("Repository"), t->GetFunction());
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_STR_ARG(0, path);

	Repository *repo = new Repository();

	if(int result = git_repository_open(&repo->repo_, *path) != GIT_SUCCESS) {
		Handle<Value> ex = Exception::Error(String::New("Git error."));
		return ThrowException(ex);
	}

	repo->path_ = *path;

	args.This()->Set(String::New("path"), String::New(repo->path_), ReadOnly);

	repo->odb_ = git_repository_database(repo->repo_);

	repo->Wrap(args.This());
	return args.This();
}

Handle<Value> Repository::GetCommit(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, commitOid);

	if(args.Length() == 2) {
		REQ_FUN_ARG(1, callbackArg);
	
		object_get_request *request = new object_get_request();
		request->callback = Persistent<Function>::New(callbackArg);
		request->repo = repo;
		
		// TODO: should just get the oid into this struct in the first place.
		// Write up some macros.
		git_oid_cpy(&request->oid, &commitOid);
		
		repo->Ref();
		
		eio_custom(EIO_GetCommit, EIO_PRI_DEFAULT, EIO_AfterGetCommit, request);
		ev_ref(EV_DEFAULT_UC);
		
		return scope.Close(Undefined());
	}
	else {
		Commit *commitObject;
		git_commit *commit;
		int res = repo->getCommit(&commitOid, &commit);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't get commit", res);
		}

		commitObject = repo->wrapCommit(commit);
		return scope.Close(commitObject->handle_);
	}
}

int Repository::EIO_GetCommit(eio_req *req) {
	GET_REQUEST_DATA();

 	git_commit* commit;
 	int result = requestData->error = requestData->repo->getCommit(&requestData->oid, &commit);
	if(result == GIT_SUCCESS) {
		requestData->object = commit;
	}
	
	return 0;
}

int Repository::EIO_AfterGetCommit(eio_req *req) {
	HandleScope scope;
	GET_REQUEST_DATA();
	 	
 	Handle<Value> *callbackArgs;
 	int argCount = 0;
 	if(requestData->error) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't get commit."), requestData->error);
 		callbackArgs = &error;
 		argCount = 1;
	}
	else {
		Commit *commitObject = requestData->repo->wrapCommit(static_cast<git_commit*>(requestData->object));
		callbackArgs = new Handle<Value>[2];
		callbackArgs[0] = Null();
		callbackArgs[1] = commitObject->handle_;
		argCount = 2;
	}
	
    TryCatch tryCatch;
	requestData->callback->Call(Context::GetCurrent()->Global(), argCount, callbackArgs);
 	if(tryCatch.HasCaught()) {
       FatalException(tryCatch);
    }
    
    REQUEST_CLEANUP();
    return 0;
}

Handle<Value> Repository::GetTree(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_OID_ARG(0, treeOid);

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_tree *tree;
	if(git_tree_lookup(&tree, repo->repo_, &treeOid) != GIT_SUCCESS) {
		return scope.Close(Null());
	}

	Tree *treeObject = repo->wrapTree(tree);
	return scope.Close(treeObject->handle_);
}

Handle<Value> Repository::GetTag(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_OID_ARG(0, tagOid);

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_tag *tag;
	int res = git_tag_lookup(&tag, repo->repo_, &tagOid);
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't get tag.", res);

	Tag *tagObj = repo->wrapTag(tag);
	return scope.Close(tagObj->handle_);
}

Handle<Value> Repository::GetRawObject(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, oid);

	git_rawobj *obj = new git_rawobj;
	int res = git_odb_read(obj, repo->odb_, &oid);
	if(res != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't load raw object.", res);
	}

	Local<Value> arg = External::New(obj);
	Persistent<Object> result(RawObject::constructor_template->GetFunction()->NewInstance(1, &arg));
	return scope.Close(result);
}

Handle<Value> Repository::CreateRawObject(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	// Initialize a new rawobj.
	git_rawobj *rawObj = new git_rawobj;
	rawObj->len = 0;
	rawObj->type = GIT_OBJ_BAD;

	Handle<Value> constructorArgs[1] = { External::New(rawObj) };
	Handle<Object> jsObject = RawObject::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

	RawObject *rawObjObj = ObjectWrap::Unwrap<RawObject>(jsObject);
	rawObjObj->repository_ = repo;
	return scope.Close(jsObject);
}

Handle<Value> Repository::CreateWalker(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_revwalk *walker;
	int res = git_revwalk_new(&walker, repo->repo_);
	if(res != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't create revision walker", res);
	}

	Handle<Value> constructorArgs[2] = { External::New(walker), External::New(repo) };
	Handle<Object> instance = RevWalker::constructor_template->GetFunction()->NewInstance(2, constructorArgs);
	
	RevWalker *walkerObject = ObjectWrap::Unwrap<RevWalker>(instance);
	return scope.Close(walkerObject->handle_);
}

Handle<Value> Repository::IndexGetter(Local<String>, const AccessorInfo& info) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(info.This());
	if(repo->index_ == NULL) {
		git_index *index;
		int result = git_repository_index(&index, repo->repo_);
		if(result == GIT_EBAREINDEX) {
			git_index_open_bare(&index, repo->path_);
		}

		Handle<Value> arg = External::New(index);
		Handle<Object> instance = Index::constructor_template->GetFunction()->NewInstance(1, &arg);
		repo->index_ = ObjectWrap::Unwrap<Index>(instance);
	}

	return repo->index_->handle_;
}

Handle<Value> Repository::CreateTag(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_tag *tag;
	int res = git_tag_new(&tag, repo->repo_);
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't create new tag.", res);

	Tag *tagObject = repo->wrapTag(tag);
	return scope.Close(tagObject->handle_);
}

Handle<Value> Repository::CreateTree(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_tree *tree;
	int res = git_tree_new(&tree, repo->repo_);
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't create tree.", res);

	Tree *treeObject = repo->wrapTree(tree);
	return treeObject->handle_;
}

Handle<Value> Repository::CreateCommit(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_commit *commit;
	int result = git_commit_new(&commit, repo->repo_);

	if(result != GIT_SUCCESS) {
		// TODO: error handling.
		return Null();
	}

	Commit *commitObject = repo->wrapCommit(commit);
	return scope.Close(commitObject->handle_);
}

Handle<Value> Repository::GetReference(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_STR_ARG(0, referenceName);

	git_reference *reference;
	int result = git_reference_lookup(&reference, repo->repo_, *referenceName);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Failed to load ref.", result);

	Reference *refObj = repo->wrapReference(reference);
	return scope.Close(refObj->handle_);
}

Handle<Value> Repository::Exists(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	REQ_ARGS(1);
	REQ_OID_ARG(0, objOid);

	return Boolean::New(git_odb_exists(repo->odb_, &objOid));
}

Repository::Repository() {
	CREATE_MUTEX(gitLock_);
}

Repository::~Repository() {
	close();
}

void Repository::close() {
	if(repo_) {
		git_repository_free(repo_);
		repo_ = NULL;
	}
}

int Repository::getCommit(git_oid *id, git_commit **cmt) {
	int result;
	
	LOCK_MUTEX(gitLock_);
	result = git_commit_lookup(cmt, repo_, id);
	UNLOCK_MUTEX(gitLock_);
	
	return result;
}

Commit *Repository::wrapCommit(git_commit *commit) {
	Commit *commitObject;
	
	if(commitStore_.getObjectFor(commit, &commitObject)) {
		// Commit needs to know who it's daddy is.
		commitObject->repository_ = this;
	}

	return commitObject;
}

Tree *Repository::wrapTree(git_tree *tree) {
	Tree *treeObject;
	if(treeStore_.getObjectFor(tree, &treeObject)) {
		treeObject->repository_ = this;
	}

	return treeObject;
}

Tag *Repository::wrapTag(git_tag *tag) {
	Tag *tagObject;
	if(tagStore_.getObjectFor(tag, &tagObject)) {
		tagObject->repository_ = this;
	}

	return tagObject;
}

Reference *Repository::wrapReference(git_reference *ref) {
	Reference *refObj;
	if(refStore_.getObjectFor(ref, &refObj)) {
		refObj->repository_ = this;
	}
}

} // namespace gitteh
