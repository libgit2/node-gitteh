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

namespace gitteh {

static Persistent<String> id_symbol;
static Persistent<String> entries_symbol;

static Persistent<String> entry_id_symbol;
static Persistent<String> entry_name_symbol;
static Persistent<String> entry_attributes_symbol;

struct tree_entry_data {
	std::string *name;
	char id[40];
	int attributes;
};

struct tree_data {
	char id[40];
	int entryCount;
	tree_entry_data **entries;
};

struct save_request {
	Persistent<Function> callback;
	Tree *tree;
	int error;
	char id[40];
};

Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	id_symbol = NODE_PSYMBOL("id");
	entries_symbol = NODE_PSYMBOL("entries");

	entry_id_symbol = NODE_PSYMBOL("id");
	entry_name_symbol = NODE_PSYMBOL("name");
	entry_attributes_symbol = NODE_PSYMBOL("attributes");
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

Handle<Value> Tree::Save(const Arguments& args) {
	HandleScope scope;

	THROW_ERROR("Not yet implemented.");
}

#ifdef FIXME
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
#endif

Tree::Tree() {
}

Tree::~Tree() {
	repository_->lockRepository();
	git_tree_close(tree_);
	repository_->unlockRepository();
}

void Tree::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	tree_data *treeData = static_cast<tree_data*>(data);

	jsObject->Set(id_symbol, String::New(treeData->id, 40),
			(PropertyAttribute)(ReadOnly | DontDelete));

	Handle<Array> entriesArray = Array::New(treeData->entryCount);
	for(int i = 0; i < treeData->entryCount; i++) {
		tree_entry_data *entryData = treeData->entries[i];
		Handle<Object> entryObject = Object::New();
		entryObject->Set(entry_id_symbol, String::New(entryData->id, 40));
		entryObject->Set(entry_name_symbol, String::New(entryData->name->c_str()));
		entryObject->Set(entry_attributes_symbol, Integer::New(entryData->attributes));

		entriesArray->Set(i, entryObject);
		delete entryData->name;
		delete entryData;
	}

	jsObject->Set(entries_symbol, entriesArray);

	delete treeData->entries;
	delete treeData;
}

void* Tree::loadInitData() {
	tree_data *data = new tree_data;

	repository_->lockRepository();
	data->entryCount = git_tree_entrycount(tree_);
	const git_oid *treeOid = git_tree_id(tree_);
	git_oid_fmt(data->id, treeOid);

	data->entries = new tree_entry_data*[data->entryCount];
	for(int i = 0; i < data->entryCount; i++) {
		git_tree_entry *gitEntry = git_tree_entry_byindex(tree_, i);
		tree_entry_data *entryData = new tree_entry_data;
		git_oid_fmt(entryData->id, git_tree_entry_id(gitEntry));
		entryData->name = new std::string(git_tree_entry_name(gitEntry));
		entryData->attributes = git_tree_entry_attributes(gitEntry);
		data->entries[i] = entryData;
	}
	repository_->unlockRepository();

	return data;
}

void Tree::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
