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
#include "signature.h"

#define CLASS_NAME String::NewSymbol("Commit")

static Persistent<String> id_symbol;
static Persistent<String> message_symbol;
static Persistent<String> author_symbol;
static Persistent<String> committer_symbol;
static Persistent<String> tree_symbol;
static Persistent<String> parents_symbol;

namespace gitteh {

struct commit_data {
	char id[40];
	std::string *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
	std::string **parentIds;
	std::string *treeId;
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
	int error;
};

struct save_commit_request {
	Persistent<Function> callback;
	Repository *repo;
	Persistent<Object> repoHandle;
	Commit *commit;
	int error;
	bool isNew;
	char id[40];
	std::string *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
	git_oid *parentIds;
	git_oid treeId;
};

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(CLASS_NAME);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	id_symbol = NODE_PSYMBOL("id");
	message_symbol = NODE_PSYMBOL("message");
	author_symbol = NODE_PSYMBOL("author");
	committer_symbol = NODE_PSYMBOL("committer");
	tree_symbol = NODE_PSYMBOL("tree");
	parents_symbol = NODE_PSYMBOL("parents");
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

static int SaveCommit() {

}

Handle<Value> Commit::SaveObject(Handle<Object> commitObject, Repository *repo,
		Handle<Value> callback, bool isNew) {
	int result, parentCount, i;
	git_oid treeId;
	git_oid *parentIds;

	HandleScope scope;

	if(!commitObject->Has(message_symbol)) {
		THROW_ERROR("Message is required.");
	}

	if(!commitObject->Has(author_symbol)) {
		THROW_ERROR("Author is required.");
	}

	if(!commitObject->Has(committer_symbol)) {
		THROW_ERROR("Committer is required.");
	}

	if(!commitObject->Has(tree_symbol)) {
		THROW_ERROR("Tree is required.");
	}

	String::Utf8Value message(commitObject->Get(message_symbol));
	if(!message.length()) {
		THROW_ERROR("Message is required.");
	}

	String::Utf8Value treeIdStr(commitObject->Get(tree_symbol));
	if(!treeIdStr.length()) {
		THROW_ERROR("Tree is required.");
	}

	treeId;
	result = git_oid_mkstr(&treeId, *treeIdStr);
	if(result != GIT_SUCCESS) {
		THROW_ERROR("Tree ID is invalid.");
	}

	Handle<Array> parents;
	if(commitObject->Has(parents_symbol)) {
		Handle<Value> parentProperty = commitObject->Get(parents_symbol);

		if(!parentProperty->IsArray()) {
			if(!parentProperty->IsNull()) {
				parents = Array::New(1);
				parents->Set(0, parentProperty);
			}
			else {
				parents = Array::New(0);
			}
		}
		else {
			parents = Local<Array>::New(Handle<Array>::Cast(parentProperty));
		}
	}

	parentCount = parents->Length();
	parentIds = new git_oid[parentCount];
	for(i = 0; i < parentCount; i++) {
		result = git_oid_mkstr(&parentIds[i], *String::Utf8Value(parents->Get(i)));
		if(result != GIT_SUCCESS) {
			delete [] parentIds;
			THROW_ERROR("Parent id is invalid.");
		}
	}

	git_signature *committer = GetSignatureFromProperty(commitObject, committer_symbol);
	if(committer == NULL) {
		delete [] parentIds;
		THROW_ERROR("Committer is not a valid signature.");
	}

	git_signature *author = GetSignatureFromProperty(commitObject, author_symbol);
	if(author == NULL) {
		delete [] parentIds;
		git_signature_free(committer);
		THROW_ERROR("Author is not a valid signature.");
	}

	// Okay, we're ready to make this happen. Are we doing it asynchronously
	// or synchronously?
	if(callback->IsFunction()) {
		save_commit_request *request = new save_commit_request;
		request->message = new std::string(*message);
		request->parentCount = parentCount;
		request->parentIds = parentIds;
		memcpy(&request->treeId, &treeId, sizeof(git_oid));
		request->committer = committer;
		request->author = author;

		request->isNew = isNew;
		if(!request->isNew) {
			request->commit = ObjectWrap::Unwrap<Commit>(commitObject);
		}
		request->callback = Persistent<Function>::New(Handle<Function>::Cast(callback));
		request->repo = repo;
		request->repoHandle = Persistent<Object>::New(repo->handle_);

		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_oid newId;

		const git_oid **parentIdsPtr;
		parentIdsPtr = new const git_oid*[parentCount];
		for(i = 0; i < parentCount; i++) {
			parentIdsPtr[i] = &parentIds[i];
		}
		result = git_commit_create(&newId, repo->repo_, NULL, author, committer,
				*message, &treeId, parentCount, parentIdsPtr);

		git_signature_free(author);
		git_signature_free(committer);
		delete [] parentIdsPtr;
		delete [] parentIds;

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't save commit.", result);
		}

		char newIdStr[40];
		git_oid_fmt(newIdStr, &newId);
		if(isNew) {
			Handle<Function> getCommitFn = Handle<Function>::Cast(
					repo->handle_->Get(String::New("getCommit")));
			Handle<Value> arg = String::New(newIdStr, 40);
			return scope.Close(getCommitFn->Call(repo->handle_, 1,
					&arg));
		}
		else {
			commitObject->Set(id_symbol, String::New(newIdStr, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));

			return scope.Close(True());
		}
	}
}

Handle<Value> Commit::Save(const Arguments& args) {
	HandleScope scope;
	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(SaveObject(args.This(), commit->repository_, callback, false));

#ifdef FIXME
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(args.This());

	CHECK_PROPERTY(MESSAGE_PROPERTY);
	Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
	if(message->Length() == 0) {
		THROW_ERROR("Message must not be empty.");
	}

	// TODO: memory leak here if committer fails, as author won't be cleaned up.
	git_signature *author = GetSignatureFromProperty(args.This(), AUTHOR_PROPERTY);
	if(author == NULL) {
		THROW_ERROR("Author property is invalid.");
	}

	git_signature *committer = GetSignatureFromProperty(args.This(), COMMITTER_PROPERTY);
	if(committer == NULL) {
		git_signature_free(author);
		THROW_ERROR("Committer property is invalid.");
	}

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
		char oidStr[40];
		git_oid_fmt(oidStr, commitId);
		commit->repository_->unlockRepository();

		args.This()->ForceSet(ID_PROPERTY, String::New(oidStr, 40), (PropertyAttribute)(ReadOnly | DontDelete));

		return True();
	}
#endif
}

int Commit::EIO_Save(eio_req *req) {
	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	const git_oid **parentIdsPtr;
	parentIdsPtr = new const git_oid*[reqData->parentCount];
	for(int i = 0; i < reqData->parentCount; i++) {
		parentIdsPtr[i] = &reqData->parentIds[i];
	}

	git_oid newId;
	reqData->repo->lockRepository();
	reqData->error = git_commit_create(&newId, reqData->repo->repo_, NULL,
			reqData->author, reqData->committer, reqData->message->c_str(),
			&reqData->treeId, reqData->parentCount, parentIdsPtr);
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		git_oid_fmt(reqData->id, &newId);
	}

	delete [] parentIdsPtr;
	delete reqData->message;
	delete [] reqData->parentIds;
	git_signature_free(reqData->committer);
	git_signature_free(reqData->author);

	return 0;
}

int Commit::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	reqData->repoHandle.Dispose();
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
		char oidStr[40];
		git_oid_fmt(oidStr, commitId);
		reqData->commit->repository_->unlockRepository();
		reqData->commit->handle_->ForceSet(id_symbol, String::New(oidStr, 40),
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
	data->parentIds = new std::string*[data->parentCount];

	for(int i = 0; i< data->parentCount; i++) {
		git_commit *parent;
		git_commit_parent(&parent, commit_, i);

		const git_oid *oid = git_commit_id(parent);
		char oidStr[40];
		git_oid_fmt(oidStr, oid);

		data->parentIds[i] = new std::string(oidStr, 40);
	}

	git_tree *commitTree;
	git_commit_tree(&commitTree, commit_);
	const git_oid *treeOid = git_tree_id(commitTree);
	char treeOidStr[40];
	git_oid_fmt(treeOidStr, treeOid);
	data->treeId = new std::string(treeOidStr, 40);

	repository_->unlockRepository();

	return data;
}

void Commit::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObj = handle_;

	if(data != NULL) {
		commit_data *commitData = static_cast<commit_data*>(data);

		jsObj->Set(id_symbol, String::New(commitData->id, 40), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(message_symbol, String::New(commitData->message->c_str()));
		delete commitData->message;

		CREATE_PERSON_OBJ(authorObj, commitData->author);
		jsObj->Set(author_symbol, authorObj);
		git_signature_free(commitData->author);

		CREATE_PERSON_OBJ(committerObj, commitData->committer);
		jsObj->Set(committer_symbol, committerObj);
		git_signature_free(commitData->committer);

		parentCount_ = commitData->parentCount;

		Handle<Array> parentsArray = Array::New(parentCount_);
		for(int i = 0; i < parentCount_; i++) {
			parentsArray->Set(i, String::New(commitData->parentIds[i]->c_str()));
			delete commitData->parentIds[i];
		}
		delete [] commitData->parentIds;
		jsObj->Set(parents_symbol, parentsArray);

		jsObj->Set(tree_symbol, String::New(commitData->treeId->c_str()));
		delete commitData->treeId;

		delete commitData;
	}
	else {
		// This is a new commit.
		jsObj->Set(id_symbol, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObj->Set(message_symbol, Null());
		jsObj->Set(author_symbol, Null());
		jsObj->Set(committer_symbol, Null());
		jsObj->Set(parents_symbol, Array::New(0));
		jsObj->Set(tree_symbol, Null());
		parentCount_ = 0;
		//jsObj->Set(PARENTCOUNT_PROPERTY, Integer::New(0), (PropertyAttribute)(ReadOnly | DontDelete));
	}
}

void Commit::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

Commit::Commit() : GitObjectWrap() {
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
}

} // namespace gitteh
