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
#include <time.h>
#include <stdlib.h>
#include "signature.h"

static Persistent<String> commit_class_symbol;
static Persistent<String> id_symbol;
static Persistent<String> message_symbol;
static Persistent<String> message_encoding_symbol;
static Persistent<String> author_symbol;
static Persistent<String> committer_symbol;
static Persistent<String> tree_symbol;
static Persistent<String> tree_id_symbol;
static Persistent<String> parents_symbol;

namespace gitteh {

struct commit_data {
	char id[40];
	string *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
	std::string **parentIds;
	std::string *treeId;
};

struct save_commit_request {
	Persistent<Function> callback;
	Repository *repo;
	Persistent<Object> repoHandle;
	Commit *commit;
	int error;
	bool isNew;
	char id[40];
	string *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
	git_oid *parentIds;
	git_oid treeId;
};

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	commit_class_symbol = NODE_PSYMBOL("Commit");
	id_symbol = NODE_PSYMBOL("id");
	message_symbol = NODE_PSYMBOL("message");
	message_encoding_symbol = NODE_PSYMBOL("messageEncoding");
	author_symbol = NODE_PSYMBOL("author");
	committer_symbol = NODE_PSYMBOL("committer");
	tree_symbol = NODE_PSYMBOL("tree");
	tree_id_symbol = NODE_PSYMBOL("treeId");
	parents_symbol = NODE_PSYMBOL("parents");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(commit_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	// NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	target->Set(commit_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;
	REQ_EXT_ARG(0, commitArg);

	Commit *commitObj = static_cast<Commit*>(commitArg->Value());
	commitObj->Wrap(args.This());

	Handle<Object> me = args.This();

	git_commit *commit = commitObj->commit_;
	char oidStr[40];
	const git_oid *oid = git_commit_id(commit);
	const git_oid *treeOid = git_commit_tree_oid(commit);

	git_oid_fmt(oidStr, oid);
	ImmutableSet(me, id_symbol, CastToJS(oidStr));
	git_oid_fmt(oidStr, treeOid);
	ImmutableSet(me, tree_id_symbol, CastToJS(oidStr));
	ImmutableSet(me, message_symbol, CastToJS(git_commit_message(commit)));
	const char *encoding = git_commit_message_encoding(commit);
	if(encoding) ImmutableSet(me, message_encoding_symbol, CastToJS(encoding));

	// commit->processInitData();

	return args.This();
}

/*
Handle<Value> Commit::SaveObject(Handle<Object> commitObject, Repository *repo,
		Handle<Value> callback, bool isNew) {
	int result, parentCount, i;
	git_oid treeId;
	git_oid *parentIds;
	git_tree *tree;

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

	result = git_oid_fromstr(&treeId, *treeIdStr);
	if(result != GIT_OK) {
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
	else {
		parents = Array::New(0);
	}

	parentCount = parents->Length();
	parentIds = new git_oid[parentCount];
	for(i = 0; i < parentCount; i++) {
		result = git_oid_fromstr(&parentIds[i], *String::Utf8Value(parents->Get(i)));
		if(result != GIT_OK) {
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

	result = git_tree_lookup(&tree, repo->repo_, &treeId);
	if(result != GIT_OK) {
		delete [] parentIds;
		git_signature_free(committer);
		git_signature_free(author);
		THROW_ERROR("Invalid tree.")
	}

	// Okay, we're ready to make this happen. Are we doing it asynchronously
	// or synchronously?
	if(callback->IsFunction()) {
		save_commit_request *request = new save_commit_request;
		request->message = new std::string(*message);
		request->parentCount = parentCount;
		request->parentIds = parentIds;
		request->treeId = treeId;
		request->committer = committer;
		request->author = author;

		request->isNew = isNew;
		if(!request->isNew) {
			request->commit = ObjectWrap::Unwrap<Commit>(commitObject);
			request->commit->Ref();
		}
		else {
			request->commit = NULL;
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

		git_commit **parentCommits;
		parentCommits = new git_commit*[parentCount];
		for(i = 0; i < parentCount; i++) {
			result = git_commit_lookup(&parentCommits[i], repo->repo_, 
				&parentIds[i]);
			if(result != GIT_OK) {
				// TODO: clean up shit here..
				THROW_ERROR("Couldn't find parent commit.");
			}
		}
		result = git_commit_create(&newId, repo->repo_, NULL, author, committer,
				 NULL, *message, tree, parentCount, 
				 const_cast<const git_commit**>(parentCommits));

		git_signature_free(author);
		git_signature_free(committer);
		delete [] parentCommits;
		delete [] parentIds;

		if(result != GIT_OK) {
			THROW_GIT_ERROR2("Couldn't save commit.");
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
			ObjectWrap::Unwrap<Commit>(commitObject)->updateCachedRef(&newId);
			commitObject->ForceSet(id_symbol, String::New(newIdStr, 40),
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
}

void Commit::EIO_Save(eio_req *req) {
	int i, result;

	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	git_commit **parentCommits;
	parentCommits = new git_commit*[reqData->parentCount];
	for(i = 0; i < reqData->parentCount; i++) {
		result = git_commit_lookup(&parentCommits[i], reqData->repo->repo_, 
			&reqData->parentIds[i]);
		if(result != GIT_OK) {
			// TODO: clean up shit here..
			return;
			//THROW_ERROR("Couldn't find parent commit.");
		}
	}

	git_oid newId;
	reqData->repo->lockRepository();
	reqData->error = git_commit_create(&newId, reqData->repo->repo_, NULL,
			reqData->author, reqData->committer, NULL, reqData->message->c_str(),
			&reqData->treeId, reqData->parentCount, 
			const_cast<const git_commit**>(parentCommits));
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_OK) {
		git_oid_fmt(reqData->id, &newId);
	}

	delete [] parentIdsPtr;
	delete reqData->message;
	delete [] reqData->parentIds;
	git_signature_free(reqData->committer);
	git_signature_free(reqData->author);
}

int Commit::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_commit_request *reqData = static_cast<save_commit_request*>(req->data);

	reqData->repoHandle.Dispose();
	ev_unref(EV_DEFAULT_UC);
 	if(reqData->commit != NULL) reqData->commit->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_OK) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't save commit."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
 		callbackArgs[0] = Null();

		if(reqData->isNew) {
			Handle<Function> getCommitFn = Handle<Function>::Cast(
					reqData->repo->handle_->Get(String::New("getCommit")));
			Handle<Value> arg = String::New(reqData->id, 40);
	 		callbackArgs[1] = getCommitFn->Call(reqData->repo->handle_, 1, &arg);
		}
		else {
			git_oid oid;
			git_oid_fromstr(&oid, reqData->id);
			reqData->commit->updateCachedRef(&oid);

			reqData->commit->handle_->ForceSet(id_symbol, String::New(reqData->id, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));
			callbackArgs[1] = True();
		}
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;
}

int Commit::doInit() {
	initData_ = new commit_data;
	repository_->lockRepository();
	const git_oid *commitId = git_commit_id(commit_);
	git_oid_fmt(initData_->id, commitId);
	initData_->message = new std::string(git_commit_message(commit_));
	initData_->author = git_signature_dup(git_commit_author(commit_));
	initData_->committer = git_signature_dup(git_commit_committer(commit_));
	initData_->parentCount = git_commit_parentcount(commit_);
	initData_->parentIds = new std::string*[initData_->parentCount];

	for(int i = 0; i< initData_->parentCount; i++) {
		git_commit *parent;
		git_commit_parent(&parent, commit_, i);

		const git_oid *oid = git_commit_id(parent);
		char oidStr[40];
		git_oid_fmt(oidStr, oid);

		initData_->parentIds[i] = new std::string(oidStr, 40);
	}

	git_tree *commitTree;
	git_commit_tree(&commitTree, commit_);
	const git_oid *treeOid = git_tree_id(commitTree);
	char treeOidStr[40];
	git_oid_fmt(treeOidStr, treeOid);
	initData_->treeId = new std::string(treeOidStr, 40);

	repository_->unlockRepository();

	//return GIT_EBUSY;
	return GIT_OK;
}

void Commit::processInitData() {
	HandleScope scope;
	Handle<Object> jsObj = handle_;

	commit_data *commitData = initData_;

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

void Commit::setOwner(Repository *owner) {
	repository_ = owner;
}*/

Commit::Commit(git_commit *commit) : GitObject((git_object*)commit) {
	commit_ = commit;
}

Commit::~Commit() {
	// repository_->lockRepository();
	// git_commit_close(commit_);
	// repository_->unlockRepository();
}
/*
void Commit::updateCachedRef(const git_oid *newId) {
	// repository_->lockRepository();
	git_commit *newCommit;
	git_commit_lookup(&newCommit, repository_->repo_, newId);
	// repository_->commitCache_->updateCacheRef(commit_, newCommit);
	// git_commit_close(commit_);
	commit_ = newCommit;
	// repository_->unlockRepository();
}*/

} // namespace gitteh
