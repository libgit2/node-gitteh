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
#include "tree.h"
#include "tree_entry.h"

#define ID_PROPERTY String::NewSymbol("id")
#define LENGTH_PROPERTY String::NewSymbol("entryCount")

#define UPDATE_ENTRY_COUNT()												\
	args.This()->ForceSet(LENGTH_PROPERTY, Integer::New(tree->entryCount_));

namespace gitteh {

struct tree_data {
	char id[40];
	int entryCount;
};

struct save_request {
	Persistent<Function> callback;
	Tree *tree;
	int error;
	char id[40];
};

struct entry_request {
	Persistent<Function> callback;
	Tree *tree;
	int index;
	std::string *name;
	git_tree_entry *entry;
	int error;
};

struct add_entry_request {
	Persistent<Function> callback;
	Tree *tree;
	git_oid id;
	int attributes;
	std::string *filename;
	int error;
	git_tree_entry *entry;
};

Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getEntry", GetEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "addEntry", AddEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "removeEntry", RemoveEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "clear", Clear);
	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTree);

	Tree *tree = new Tree();
	tree->Wrap(args.This());

	tree->tree_ = (git_tree*)theTree->Value();

	return args.This();
}

Handle<Value> Tree::GetEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	if(HAS_CALLBACK_ARG) {
		entry_request *request = new entry_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->tree = tree;
		request->callback = Persistent<Function>::New(callbackArg);
		request->entry = NULL;

		if(args[0]->IsString()) {
			REQ_STR_ARG(0, propertyName);
			request->name = new std::string(*propertyName);
		}
		else {
			REQ_INT_ARG(0, indexArg);
			request->index = indexArg;
			request->name = NULL;
		}

		tree->Ref();
		eio_custom(EIO_GetEntry, EIO_PRI_DEFAULT, EIO_AfterGetEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_tree_entry *entry;

		if(args[0]->IsString()) {
			REQ_STR_ARG(0, propertyName);

			tree->repository_->lockRepository();
			entry = git_tree_entry_byname(tree->tree_, const_cast<const char*>(*propertyName));
			tree->repository_->unlockRepository();
		}
		else {
			REQ_INT_ARG(0, indexArg);
			tree->repository_->lockRepository();
			entry = git_tree_entry_byindex(tree->tree_, indexArg);
			tree->repository_->unlockRepository();
		}

		if(entry == NULL) {
			return scope.Close(Null());
		}

		return scope.Close(tree->entryFactory_->syncRequestObject(entry)->handle_);
	}
}

int Tree::EIO_GetEntry(eio_req *req) {
	entry_request *reqData = static_cast<entry_request*>(req->data);

	reqData->tree->repository_->lockRepository();
	if(reqData->name != NULL) {
		reqData->entry = git_tree_entry_byname(reqData->tree->tree_,
				reqData->name->c_str());
		delete reqData->name;
	}
	else {
		reqData->entry = git_tree_entry_byindex(reqData->tree->tree_, reqData->index);
	}
	reqData->tree->repository_->unlockRepository();

	return 0;
}

int Tree::EIO_AfterGetEntry(eio_req *req) {
	HandleScope scope;
	entry_request *reqData = static_cast<entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tree->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->entry == NULL) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't get tree entry."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();

 		TRIGGER_CALLBACK();
 		reqData->callback.Dispose();
	}
	else {
		reqData->tree->entryFactory_->asyncRequestObject(
				reqData->entry, reqData->callback);
	}

	delete reqData;
	return 0;
}

Handle<Value> Tree::AddEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(3);
	REQ_OID_ARG(0, idArg);
	REQ_STR_ARG(1, filenameArg);
	REQ_INT_ARG(2, modeArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	if(HAS_CALLBACK_ARG) {
		add_entry_request *request = new add_entry_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->tree = tree;
		request->callback = Persistent<Function>::New(callbackArg);

		memcpy(&request->id, &idArg, sizeof(git_oid));
		request->filename = new std::string(*filenameArg);
		request->attributes = modeArg;

		tree->Ref();
		eio_custom(EIO_AddEntry, EIO_PRI_DEFAULT, EIO_AfterAddEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		git_tree_entry *entry;
		tree->repository_->lockRepository();
		int res = git_tree_add_entry(&entry, tree->tree_, &idArg, *filenameArg, modeArg);
		tree->repository_->unlockRepository();

		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Error creating tree entry.", res);
		}

		tree->entryCount_++;
		UPDATE_ENTRY_COUNT();

		return scope.Close(tree->entryFactory_->syncRequestObject(entry)->handle_);
	}
}

int Tree::EIO_AddEntry(eio_req *req) {
	add_entry_request *reqData = static_cast<add_entry_request*>(req->data);

	reqData->tree->repository_->lockRepository();
	reqData->error = git_tree_add_entry(&reqData->entry, reqData->tree->tree_,
			&reqData->id, reqData->filename->c_str(), reqData->attributes);
	reqData->tree->repository_->unlockRepository();

	delete reqData->filename;

	return 0;
}

int Tree::EIO_AfterAddEntry(eio_req *req) {
	HandleScope scope;

	add_entry_request *reqData = static_cast<add_entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tree->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't add tree entry."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();

 		TRIGGER_CALLBACK();
 		reqData->callback.Dispose();
	}
	else {
		reqData->tree->entryFactory_->asyncRequestObject(
				reqData->entry, reqData->callback);
	}

	delete reqData;
	return 0;
}

Handle<Value> Tree::RemoveEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, indexArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		entry_request *request = new entry_request;

		request->tree = tree;
		request->callback = Persistent<Function>::New(callbackArg);

		if(args[0]->IsString()) {
			REQ_STR_ARG(0, propertyName);
			request->name = new std::string(*propertyName);
		}
		else {
			REQ_INT_ARG(0, indexArg);
			request->index = indexArg;
			request->name = NULL;
		}

		tree->Ref();
		eio_custom(EIO_RemoveEntry, EIO_PRI_DEFAULT, EIO_AfterRemoveEntry, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		int res;
		git_tree_entry *entry;

		if(args[0]->IsString()) {
			REQ_STR_ARG(0, propertyName);

			tree->repository_->lockRepository();
			entry = git_tree_entry_byname(tree->tree_, *propertyName);
			tree->repository_->unlockRepository();

			if(!entry) {
				THROW_ERROR("Entry with that name not found.");
			}

			tree->repository_->lockRepository();
			res = git_tree_remove_entry_byname(tree->tree_, *propertyName);
			tree->repository_->unlockRepository();
		}
		else {
			REQ_INT_ARG(0, indexArg);

			tree->repository_->lockRepository();
			entry = git_tree_entry_byindex(tree->tree_, indexArg);
			tree->repository_->unlockRepository();

			if(!entry) {
				THROW_ERROR("Entry with that index not found.");
			}

			tree->repository_->lockRepository();
			res = git_tree_remove_entry_byindex(tree->tree_, indexArg);
			tree->repository_->unlockRepository();
		}

		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't delete tree entry", res);
		}

		tree->entryFactory_->deleteObject(entry);

		tree->entryCount_--;
		UPDATE_ENTRY_COUNT();

		return scope.Close(True());
	}
}

int Tree::EIO_RemoveEntry(eio_req *req) {
	entry_request *reqData = static_cast<entry_request*>(req->data);

	reqData->tree->repository_->lockRepository();
	if(reqData->name != NULL) {
		reqData->entry = git_tree_entry_byname(reqData->tree->tree_,
				reqData->name->c_str());

		if(reqData->entry == NULL) {
			reqData->error = GIT_ENOTFOUND;
		}
		else {
			reqData->error = git_tree_remove_entry_byname(reqData->tree->tree_,
					reqData->name->c_str());
		}

		delete reqData->name;
	}
	else {
		reqData->entry = git_tree_entry_byindex(reqData->tree->tree_,
				reqData->index);

		if(reqData->entry == NULL) {
			reqData->error = GIT_ENOTFOUND;
		}
		else {
			reqData->error = git_tree_remove_entry_byindex(reqData->tree->tree_,
					reqData->index);
		}
	}

	reqData->tree->repository_->unlockRepository();

	// TODO: what happens if someone tries to do something with an entry at this
	// point? We might need a mutex for tree entries...

	return 0;
}

int Tree::EIO_AfterRemoveEntry(eio_req *req) {
	HandleScope scope;

	add_entry_request *reqData = static_cast<add_entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tree->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't add tree entry."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		// Note for future me: in this situation, reqData->entry ptr is actually
		// pointing to a non-existent entry, so technically if we were to do
		// anything with it we'd most likely segfault. However! All we're doing
		// here is asking the object factory (and consequently the objectstore)
		// to delete the js object that belongs to this pointer. If we ever
		// made the objectstore work off anything other than ptr address,
		reqData->tree->entryFactory_->deleteObject(reqData->entry);
		reqData->tree->entryCount_--;
		reqData->tree->handle_->ForceSet(LENGTH_PROPERTY, Integer::New(reqData->tree->entryCount_),
				(PropertyAttribute)(ReadOnly | DontDelete));

 		callbackArgs[0] = Null();
 		callbackArgs[1] = True();
	}

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();

	delete reqData;
	return 0;
}

Handle<Value> Tree::Clear(const Arguments& args) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	if(HAS_CALLBACK_ARG) {
		entry_request *request = new entry_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->tree = tree;
		request->callback = Persistent<Function>::New(callbackArg);

		tree->Ref();
		eio_custom(EIO_ClearEntries, EIO_PRI_DEFAULT, EIO_AfterClearEntries, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		tree->repository_->lockRepository();
		git_tree_clear_entries(tree->tree_);
		tree->repository_->unlockRepository();

		// TODO: could probably implement something in objectstore/objectfactory to
		// make this a little more ... elegant.
		delete tree->entryFactory_;
		tree->entryFactory_ = new ObjectFactory<Tree, TreeEntry, git_tree_entry>(tree);

		tree->entryCount_ = 0;
		UPDATE_ENTRY_COUNT();

		return scope.Close(Undefined());
	}
}

int Tree::EIO_ClearEntries(eio_req *req) {
	entry_request *reqData = static_cast<entry_request*>(req->data);

	reqData->tree->repository_->lockRepository();
	git_tree_clear_entries(reqData->tree->tree_);
	reqData->tree->repository_->unlockRepository();

	return 0;
}

int Tree::EIO_AfterClearEntries(eio_req *req) {
	HandleScope scope;
	entry_request *reqData = static_cast<entry_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tree->Unref();

	delete reqData->tree->entryFactory_;
	reqData->tree->entryFactory_ = new ObjectFactory<Tree, TreeEntry, git_tree_entry>(reqData->tree);

	reqData->tree->entryCount_ = 0;
	reqData->tree->handle_->ForceSet(LENGTH_PROPERTY, Integer::New(reqData->tree->entryCount_),
			(PropertyAttribute)(ReadOnly | DontDelete));

	Handle<Value> callbackArgs[2];
	callbackArgs[0] = Null();
	callbackArgs[1] = True();

	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;

	return 0;
}

Handle<Value> Tree::Save(const Arguments& args) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	if(HAS_CALLBACK_ARG) {
		save_request *request = new save_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);

		request->tree = tree;
		request->callback = Persistent<Function>::New(callbackArg);

		tree->Ref();
		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		tree->repository_->lockRepository();
		int result = git_object_write((git_object *)tree->tree_);
		tree->repository_->unlockRepository();
		if(result != GIT_SUCCESS) {
			return ThrowException(CreateGitError(String::New("Error saving tree."), result));
		}

		tree->repository_->lockRepository();
		const git_oid *treeOid = git_tree_id(tree->tree_);
		tree->repository_->unlockRepository();
		char oidStr[40];
		git_oid_fmt(oidStr, treeOid);
		args.This()->ForceSet(String::New("id"), String::New(oidStr, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

		return Undefined();
	}
}

int Tree::EIO_Save(eio_req *req) {
	save_request *reqData = static_cast<save_request*>(req->data);

	reqData->tree->repository_->lockRepository();
	reqData->error = git_object_write((git_object *)reqData->tree->tree_);

	if(reqData->error == GIT_SUCCESS) {
		const git_oid *treeOid = git_tree_id(reqData->tree->tree_);
		git_oid_fmt(reqData->id, treeOid);
	}

	reqData->tree->repository_->unlockRepository();

	return 0;
}

int Tree::EIO_AfterSave(eio_req *req) {
	HandleScope scope;

	save_request *reqData = static_cast<save_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tree->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = Exception::Error(String::New("Couldn't save tree."));
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->tree->handle_->ForceSet(String::New("id"),String::New(reqData->id, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

 		callbackArgs[0] = Null();
 		callbackArgs[1] = True();
	}


	TRIGGER_CALLBACK();
	reqData->callback.Dispose();
	delete reqData;
	return 0;
}

Tree::Tree() {
	entryFactory_ = new ObjectFactory<Tree, TreeEntry, git_tree_entry>(this);
}

Tree::~Tree() {
	delete entryFactory_;
}

void Tree::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	if(data != NULL) {
		tree_data *treeData = static_cast<tree_data*>(data);
		entryCount_ = treeData->entryCount;

		jsObject->Set(ID_PROPERTY, String::New(treeData->id, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

		jsObject->Set(LENGTH_PROPERTY, Integer::New(treeData->entryCount),
				(PropertyAttribute)(ReadOnly | DontDelete));

		delete treeData;
	}
	else {
		entryCount_ = 0;

		jsObject->Set(ID_PROPERTY, Null(), ReadOnly);
		jsObject->Set(LENGTH_PROPERTY, Integer::New(0),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}

}

void* Tree::loadInitData() {
	tree_data *data = new tree_data;

	repository_->lockRepository();
	data->entryCount = git_tree_entrycount(tree_);
	const git_oid *treeOid = git_tree_id(tree_);
	git_oid_fmt(data->id, treeOid);
	repository_->unlockRepository();

	return data;
}

void Tree::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
