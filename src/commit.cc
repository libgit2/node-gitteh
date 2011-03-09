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
#include "tree.h"
#include <time.h>

#define ID_PROPERTY String::NewSymbol("id")
#define MESSAGE_PROPERTY String::NewSymbol("message")
#define TIME_PROPERTY String::NewSymbol("time")
#define AUTHOR_PROPERTY String::NewSymbol("author")
#define COMMITTER_PROPERTY String::NewSymbol("committer")
#define TREE_PROPERTY String::NewSymbol("tree")

namespace gitteh {

struct get_parent_request {
	Persistent<Function> callback;
	Commit *commit;
	int index;
	git_commit *parent;
};

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
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

	// Setup some basic info about this commit.
	const git_oid *commitId = git_commit_id(commit->commit_);

	Handle<Object> jsObj = args.This();
	if(commitId) {
		const char* oidStr = git_oid_allocfmt(commitId);

		jsObj->Set(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));
		const char* message = git_commit_message(commit->commit_);
		jsObj->Set(MESSAGE_PROPERTY, String::New(message));

		const git_signature *author;
		author = git_commit_author(commit->commit_);
		if(author) {
			CREATE_PERSON_OBJ(authorObj, author);
			jsObj->Set(AUTHOR_PROPERTY, authorObj);
		}

		const git_signature *committer;
		committer = git_commit_committer(commit->commit_);
		if(committer) {
			CREATE_PERSON_OBJ(committerObj, committer);
			jsObj->Set(COMMITTER_PROPERTY, committerObj);
		}

		commit->parentCount_ = git_commit_parentcount(commit->commit_);

		jsObj->Set(String::New("parentCount"), Integer::New(commit->parentCount_), (PropertyAttribute)(ReadOnly | DontDelete));
	}
	else {
		// This is a new commit.
		jsObj->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(MESSAGE_PROPERTY, Null());
		jsObj->Set(AUTHOR_PROPERTY, Null());
		jsObj->Set(COMMITTER_PROPERTY, Null());
		commit->parentCount_ = 0;
		jsObj->Set(String::New("parentCount"), Integer::New(0), (PropertyAttribute)(ReadOnly | DontDelete));
	}

	commit->Wrap(args.This());
	return args.This();
}

Handle<Value> Commit::GetTree(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	const git_tree *tree = git_commit_tree(commit->commit_);
	if(tree == NULL) {
		return scope.Close(Null());
	}

	Tree *treeObject = commit->repository_->wrapTree(const_cast<git_tree*>(tree));
	return scope.Close(treeObject->handle_);
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

		res = git_tree_lookup(&tree, commit->repository_->repo_, &treeId);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error locating tree", res);
		}
	}
	else if(Tree::constructor_template->HasInstance(args[0])) {
		Tree *treeObj = ObjectWrap::Unwrap<Tree>(Handle<Object>::Cast(args[0]));
		tree = treeObj->tree_;
	}

	git_commit_set_tree(commit->commit_, tree);
}

Handle<Value> Commit::GetParent(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	REQ_ARGS(1);
	REQ_INT_ARG(0, indexArg);

	if(indexArg >= commit->parentCount_) {
		THROW_ERROR("Parent commit index is out of bounds.");
	}

	if(args.Length() > 1) {
		get_parent_request *request = new get_parent_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->commit = commit;
		request->callback = Persistent<Function>::New(callbackArg);
		request->index = indexArg;

		eio_custom(EIO_GetParent, EIO_PRI_DEFAULT, EIO_AfterGetParent, request);
		commit->Ref();
		ev_ref(EV_DEFAULT_UC);
	}
	else {
		git_commit *parent = commit->repository_->getParentCommit(commit->commit_, indexArg);
		Commit *parentObject = commit->repository_->wrapCommit(parent);
		return scope.Close(parentObject->handle_);
	}
}

int Commit::EIO_GetParent(eio_req *req) {
	get_parent_request *reqData = static_cast<get_parent_request*>(req->data);

	reqData->parent = reqData->commit->repository_->getParentCommit(
			reqData->commit->commit_, reqData->index);

	return 0;
}

int Commit::EIO_AfterGetParent(eio_req *req) {
	HandleScope scope;
	get_parent_request *reqData = static_cast<get_parent_request*>(req->data);

	Handle<Value> callbackArgs[2];
 	if(reqData->parent == NULL) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't get parent commit."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		Commit *object = reqData->commit->repository_->wrapCommit(reqData->parent);
		callbackArgs[0] = Null();
		callbackArgs[1] = object->handle_;
	}

 	TRIGGER_CALLBACK();

	ev_unref(EV_DEFAULT_UC);
	reqData->callback.Dispose();
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

	git_commit_add_parent(commit->commit_, parentCommit);

	args.This()->ForceSet(String::New("parentCount"), Integer::New(++commit->parentCount_), (PropertyAttribute)(ReadOnly | DontDelete));

	return scope.Close(Undefined());
}

Handle<Value> Commit::Save(const Arguments& args) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	CHECK_PROPERTY(MESSAGE_PROPERTY);
	Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
	if(message->Length() == 0) {
		THROW_ERROR("Message must not be empty.");
	}

	GET_SIGNATURE_PROPERTY(AUTHOR_PROPERTY, author);
	GET_SIGNATURE_PROPERTY(COMMITTER_PROPERTY, committer);

	git_commit_set_message(commit->commit_, *String::Utf8Value(message));
	git_commit_set_committer(commit->commit_, committer);
	git_commit_set_author(commit->commit_, author);

	int result = git_object_write((git_object *)commit->commit_);
	if(result != GIT_SUCCESS) {
		return ThrowException(Exception::Error(String::New("Failed to save commit object.")));
	}

	const git_oid *commitId = git_commit_id(commit->commit_);
	const char* oidStr = git_oid_allocfmt(commitId);
	args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

	return True();
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}

} // namespace gitteh
