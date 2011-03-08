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

	args.This()->ForceSet(ID_PROPERTY, String::New(git_oid_allocfmt(git_tree_entry_id(entry->entry_))));
	args.This()->ForceSet(ATTRIBUTES_PROPERTY, Integer::New(git_tree_entry_attributes(entry->entry_)));
	args.This()->ForceSet(FILENAME_PROPERTY, String::New(git_tree_entry_name(entry->entry_)));

	entry->Wrap(args.This());
	return args.This();
}

Handle<Value> TreeEntry::SetterHandler(Local<String> property, Local<Value> value, const AccessorInfo& info) {
	HandleScope scope;

	TreeEntry *entry = ObjectWrap::Unwrap<TreeEntry>(info.This());

	if(property->Equals(ID_PROPERTY)) {
		git_oid newId;
		int res = git_oid_mkstr(&newId, *String::Utf8Value(value->ToString()));

		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Failed to parse new id", res);
		}

		git_tree_entry_set_id(entry->entry_, &newId);
	}
	else if(property->Equals(ATTRIBUTES_PROPERTY)) {
		git_tree_entry_set_attributes(entry->entry_, value->Uint32Value());
	}
	else if(property->Equals(FILENAME_PROPERTY)) {
		git_tree_entry_set_name(entry->entry_, *String::Utf8Value(value->ToString()));
	}

	return scope.Close(Handle<Value>());
}

TreeEntry::~TreeEntry() {
}

} // namespace gitteh
