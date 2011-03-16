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

#include "commit.h"
#include "repository.h"
#include "object_factory.h"
#include "tree.h"
#include <time.h>
#include <stdlib.h>

#define CLASS_NAME String::NewSymbol("Commit")

#define ID_PROPERTY String::NewSymbol("id")
#define MESSAGE_PROPERTY String::NewSymbol("message")
#define TIME_PROPERTY String::NewSymbol("time")
#define AUTHOR_PROPERTY String::NewSymbol("author")
#define COMMITTER_PROPERTY String::NewSymbol("committer")
#define TREE_PROPERTY String::NewSymbol("tree")
#define PARENTCOUNT_PROPERTY String::NewSymbol("parentCount")

namespace gitteh {

struct commit_data {
	char id[40];
	std::string *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
};

struct parent_request {
	Persistent<Function> callback;
	Commit *commit;
	int index;
	git_commit *parent;
	int error;
};

struct tree_request {
	Persistent<Function> callback;
	Commit *commit;
	git_tree *tree;
};

struct save_commit_request {
	Persistent<Function> callback;
	Commit *commit;
	int error;
	std::string *message;
	git_signature *author;
	git_signature *committer;
};

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(CLASS_NAME);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "setTree", SetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "addParent", AddParent);
	NODE_SET_PROTOTYPE_METHOD(t, "getParent", GetParent);
	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();
	commit->Wrap(args.This());

	return args.This();
}

Handle<Value> Commit::GetTree(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	// TODO: You know... we could cache the js object once we've got it...

	if(HAS_CALLBACK_ARG) {
		tree_request *request = new tree_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);

		commit->Ref();
		eio_custom(EIO_GetTree, EIO_PRI_DEFAULT, EIO_AfterGetTree, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		commit->repository_->lockRepository();
		const git_tree *tree = git_commit_tree(commit->commit_);
		commit->repository_->unlockRepository();
		if(tree == NULL) {
			return scope.Close(Null());
		}

		return commit->repository_->treeFactory_->
				syncRequestObject(const_cast<git_tree*>(tree))->handle_;
	}
}

int Commit::EIO_GetTree(eio_req *req) {
	tree_request *reqData = static_cast<tree_request*>(req->data);

	reqData->commit->repository_->lockRepository();
	reqData->tree = const_cast<git_tree*>(git_commit_tree(reqData->commit->commit_));
	reqData->commit->repository_->unlockRepository();

	return 0;
}

int Commit::EIO_AfterGetTree(eio_req *req) {
	HandleScope scope;
	tree_request *reqData = static_cast<tree_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->tree == NULL) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't get tree."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();

 		TRIGGER_CALLBACK();

 		reqData->callback.Dispose();
	}
	else {
		reqData->commit->repository_->treeFactory_->asyncRequestObject(
				reqData->tree, reqData->callback);
	}

	delete reqData;

	return 0;
}

Handle<Value> Commit::SetTree(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	git_tree *tree;
	if(args[0]->IsString()) {
		git_oid treeId;
		int res = git_oid_mkstr(&treeId, *String::Utf8Value(args[0]));
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Id is invalid", res);
		}

		commit->repository_->lockRepository();
		res = git_tree_lookup(&tree, commit->repository_->repo_, &treeId);
		commit->repository_->unlockRepository();

		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error locating tree", res);
		}
	}
	else if(Tree::constructor_template->HasInstance(args[0])) {
		Tree *treeObj = ObjectWrap::Unwrap<Tree>(Handle<Object>::Cast(args[0]));
		tree = treeObj->tree_;
	}

	if(HAS_CALLBACK_ARG) {
		tree_request *request = new tree_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);
		request->tree = tree;

		commit->Ref();
		eio_custom(EIO_SetTree, EIO_PRI_DEFAULT, EIO_AfterSetTree, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		commit->repository_->lockRepository();
		git_commit_set_tree(commit->commit_, tree);
		commit->repository_->unlockRepository();
	}

	return Undefined();
}

int Commit::EIO_SetTree(eio_req *req) {
	tree_request *reqData = static_cast<tree_request*>(req->data);

	reqData->commit->repository_->lockRepository();
	git_commit_set_tree(reqData->commit->commit_, reqData->tree);
	reqData->commit->repository_->unlockRepository();

	return 0;
}

int Commit::EIO_AfterSetTree(eio_req *req) {
	HandleScope scope;
	tree_request *reqData = static_cast<tree_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
	reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
	callbackArgs[0] = Null();
	callbackArgs[1] = True();
	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> Commit::GetParent(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	REQ_ARGS(1);
	REQ_INT_ARG(0, indexArg);

	if(indexArg >= commit->parentCount_) {
		THROW_ERROR("Parent commit index is out of bounds.");
	}

	if(HAS_CALLBACK_ARG) {
		parent_request *request = new parent_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);
		request->index = indexArg;

		commit->Ref();
		eio_custom(EIO_GetParent, EIO_PRI_DEFAULT, EIO_AfterGetParent, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		commit->repository_->lockRepository();
		git_commit *parent = git_commit_parent(commit->commit_, indexArg);
		commit->repository_->unlockRepository();


		if(parent == NULL) {
			THROW_ERROR("Error getting parent.");
		}

		Commit *parentObject = commit->repository_->commitFactory_->syncRequestObject(parent);
		return scope.Close(parentObject->handle_);
	}
}

int Commit::EIO_GetParent(eio_req *req) {
	parent_request *reqData = static_cast<parent_request*>(req->data);

	reqData->commit->repository_->lockRepository();
	reqData->parent = git_commit_parent(reqData->commit->commit_, reqData->index);
	reqData->commit->repository_->unlockRepository();

	return 0;
}

int Commit::EIO_AfterGetParent(eio_req *req) {
	HandleScope scope;
	parent_request *reqData = static_cast<parent_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->parent == NULL) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't get parent commit."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();

 		TRIGGER_CALLBACK();

 		reqData->callback.Dispose();
	}
	else {
		reqData->commit->repository_->commitFactory_->asyncRequestObject(
				reqData->parent, reqData->callback);
	}

	delete reqData;

	return 0;
}

Handle<Value> Commit::AddParent(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	REQ_ARGS(1);

	git_commit *parentCommit;
	if(args[0]->IsString()) {
		git_oid commitId;
		int res = git_oid_mkstr(&commitId, *String::Utf8Value(args[0]));
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Id is invalid", res);
		}

		res = git_commit_lookup(&parentCommit, commit->repository_->repo_, &commitId);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error locating commit", res);
		}
	}
	else if(constructor_template->HasInstance(args[0])) {
		Commit *otherCommit = ObjectWrap::Unwrap<Commit>(Handle<Object>::Cast(args[0]));
		parentCommit = otherCommit->commit_;
	}
	else {
		THROW_ERROR("Invalid argument.");
	}

	if(HAS_CALLBACK_ARG) {
		parent_request *request = new parent_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);
		request->parent = parentCommit;

		commit->Ref();
		eio_custom(EIO_AddParent, EIO_PRI_DEFAULT, EIO_AfterAddParent, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		commit->repository_->lockRepository();
		int result = git_commit_add_parent(commit->commit_, parentCommit);
		commit->repository_->unlockRepository();
		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't add parent.", result);
		}
		args.This()->ForceSet(String::New("parentCount"), Integer::New(++commit->parentCount_), (PropertyAttribute)(ReadOnly | DontDelete));
	}

	return scope.Close(Undefined());
}

int Commit::EIO_AddParent(eio_req *req) {
	parent_request *reqData = static_cast<parent_request*>(req->data);

	reqData->commit->repository_->lockRepository();
	reqData->error = git_commit_add_parent(reqData->commit->commit_, reqData->parent);
	reqData->commit->repository_->unlockRepository();

	return 0;
}

int Commit::EIO_AfterAddParent(eio_req *req) {
	HandleScope scope;
	parent_request *reqData = static_cast<parent_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't add parent commit."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->commit->handle_->ForceSet(String::New("parentCount"),
				Integer::New(++reqData->commit->parentCount_),
				(PropertyAttribute)(ReadOnly | DontDelete));

 		callbackArgs[0] = Null();
 		callbackArgs[1] = True();
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

Handle<Value> Commit::Save(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	CHECK_PROPERTY(MESSAGE_PROPERTY);
	Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
	if(message->Length() == 0) {
		THROW_ERROR("Message must not be empty.");
	}

	// TODO: memory leak here if committer fails, as author won't be cleaned up.
	GET_SIGNATURE_PROPERTY(AUTHOR_PROPERTY, author);
	GET_SIGNATURE_PROPERTY(COMMITTER_PROPERTY, committer);

	if(HAS_CALLBACK_ARG) {
		save_commit_request *request = new save_commit_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);
		request->author = author;
		request->committer = committer;
		request->message = new std::string(*String::Utf8Value(message));

		commit->Ref();
		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		commit->repository_->lockRepository();
		git_commit_set_message(commit->commit_, *String::Utf8Value(message));
		git_commit_set_committer(commit->commit_, committer);
		git_commit_set_author(commit->commit_, author);

		int result = git_object_write((git_object *)commit->commit_);

		git_signature_free(committer);
		git_signature_free(author);

		commit->repository_->unlockRepository();

		if(result != GIT_SUCCESS) {
			return ThrowException(Exception::Error(String::New("Failed to save commit object.")));
		}

		commit->repository_->lockRepository();
		const git_oid *commitId = git_commit_id(commit->commit_);
		const char* oidStr = git_oid_allocfmt(commitId);
		commit->repository_->unlockRepository();

		args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

		return True();
	}
}

int Commit::EIO_Save(eio_req *req) {
	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	reqData->commit->repository_->lockRepository();
	git_commit_set_message(reqData->commit->commit_, reqData->message->c_str());
	git_commit_set_committer(reqData->commit->commit_, reqData->committer);
	git_commit_set_author(reqData->commit->commit_, reqData->author);
	reqData->error = git_object_write((git_object *)reqData->commit->commit_);
	reqData->commit->repository_->unlockRepository();

	delete reqData->message;
	git_signature_free(reqData->committer);
	git_signature_free(reqData->author);

	return 0;
}

int Commit::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't save commit."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->commit->repository_->lockRepository();
		const git_oid *commitId = git_commit_id(reqData->commit->commit_);
		const char* oidStr = git_oid_allocfmt(commitId);
		reqData->commit->repository_->unlockRepository();
		reqData->commit->handle_->ForceSet(ID_PROPERTY, String::New(oidStr),
				(PropertyAttribute)(ReadOnly | DontDelete));

 		callbackArgs[0] = Null();
 		callbackArgs[1] = True();
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

void* Commit::loadInitData() {
	commit_data *data = new commit_data;
	repository_->lockRepository();
	const git_oid *commitId = git_commit_id(commit_);
	git_oid_fmt(data->id, commitId);
	data->message = new std::string(git_commit_message(commit_));
	data->author = git_signature_dup(git_commit_author(commit_));
	data->committer = git_signature_dup(git_commit_committer(commit_));
	data->parentCount = git_commit_parentcount(commit_);

	repository_->unlockRepository();

	return data;
}

void Commit::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObj = handle_;

	if(data != NULL) {
		commit_data *commitData = static_cast<commit_data*>(data);

		jsObj->Set(ID_PROPERTY, String::New(commitData->id, 40), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(MESSAGE_PROPERTY, String::New(commitData->message->c_str()));

		CREATE_PERSON_OBJ(authorObj, commitData->author);
		jsObj->Set(AUTHOR_PROPERTY, authorObj);

		CREATE_PERSON_OBJ(committerObj, commitData->committer);
		jsObj->Set(COMMITTER_PROPERTY, committerObj);

		parentCount_ = commitData->parentCount;
		jsObj->Set(PARENTCOUNT_PROPERTY, Integer::New(parentCount_), (PropertyAttribute)(ReadOnly | DontDelete));

		git_signature_free(commitData->author);
		git_signature_free(commitData->committer);
		delete commitData->message;
		delete commitData;
	}
	else {
		// This is a new commit.
		jsObj->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(MESSAGE_PROPERTY, Null());
		jsObj->Set(AUTHOR_PROPERTY, Null());
		jsObj->Set(COMMITTER_PROPERTY, Null());
		parentCount_ = 0;
		jsObj->Set(PARENTCOUNT_PROPERTY, Integer::New(0), (PropertyAttribute)(ReadOnly | DontDelete));
	}
}

void Commit::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

Commit::Commit() : ThreadSafeObjectWrap() {
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}

} // namespace gitteh
