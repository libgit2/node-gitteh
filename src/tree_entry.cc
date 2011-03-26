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

#include "tree.h"
#include "tree_entry.h"

#define ID_PROPERTY String::NewSymbol("id")
#define ATTRIBUTES_PROPERTY String::NewSymbol("attributes")
#define FILENAME_PROPERTY String::NewSymbol("filename")

namespace gitteh {

Persistent<FunctionTemplate> TreeEntry::constructor_template;

void TreeEntry::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("TreeEntry"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->InstanceTemplate()->SetNamedPropertyHandler(0, SetterHandler);
}

Handle<Value> TreeEntry::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theEntry);

	TreeEntry *entry = new TreeEntry();
	entry->entry_ = (git_tree_entry*)theEntry->Value();

	entry->Wrap(args.This());
	return args.This();
}

Handle<Value> TreeEntry::SetterHandler(Local<String> property, Local<Value> value, const AccessorInfo& info) {
#ifdef FIXME
	HandleScope scope;

	TreeEntry *entry = ObjectWrap::Unwrap<TreeEntry>(info.This());
	int result = GIT_SUCCESS;

	if(property->Equals(ID_PROPERTY)) {
		git_oid newId;
		result = git_oid_mkstr(&newId, *String::Utf8Value(value->ToString()));

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Failed to parse new id", result);
		}

		entry->tree_->repository_->lockRepository();
		git_tree_entry_set_id(entry->entry_, &newId);
		entry->tree_->repository_->unlockRepository();
	}
	else if(property->Equals(ATTRIBUTES_PROPERTY)) {
		entry->tree_->repository_->lockRepository();
		result = git_tree_entry_set_attributes(entry->entry_, value->Uint32Value());
		entry->tree_->repository_->unlockRepository();
	}
	else if(property->Equals(FILENAME_PROPERTY)) {
		entry->tree_->repository_->lockRepository();
		git_tree_entry_set_name(entry->entry_, *String::Utf8Value(value->ToString()));
		entry->tree_->repository_->unlockRepository();
	}

	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Failed to set property.", result);
	}
	return scope.Close(Handle<Value>());
#endif
}

TreeEntry::~TreeEntry() {

}

struct tree_entry_data {
	char id[40];
	int attributes;
	std::string *filename;
};

void *TreeEntry::loadInitData() {
	tree_entry_data *data = new tree_entry_data;

	tree_->repository_->lockRepository();
	const git_oid *entryId = git_tree_entry_id(entry_);
	git_oid_fmt(data->id, entryId);
	data->filename = new std::string(git_tree_entry_name(entry_));
	data->attributes = git_tree_entry_attributes(entry_);
	tree_->repository_->unlockRepository();

	return data;
}

void TreeEntry::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;
	tree_entry_data *reqData = static_cast<tree_entry_data*>(data);

	jsObject->ForceSet(ID_PROPERTY, String::New(reqData->id, 40));
	jsObject->ForceSet(ATTRIBUTES_PROPERTY, Integer::New(reqData->attributes));
	jsObject->ForceSet(FILENAME_PROPERTY, String::New(reqData->filename->c_str()));

	delete reqData->filename;
	delete reqData;
}

void TreeEntry::setOwner(void *owner) {
	tree_ = static_cast<Tree*>(owner);
}

} // namespace gitteh
