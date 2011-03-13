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
#define LENGTH_PROPERTY String::NewSymbol("entryCount")

#define UPDATE_ENTRY_COUNT()							\
	args.This()->ForceSet(LENGTH_PROPERTY, Integer::New(tree->entryCount_));

#define RETURN_WRAP_TREE_ENTRY()							\
	TreeEntry *treeEntryObject = tree->wrapEntry(entry);	\
	return scope.Close(treeEntryObject->handle_);

namespace gitteh {

Persistent<FunctionTemplate> Tree::constructor_template;

// TODO: I think I'm going about this all wrong. I'm trying to lock down the tree entries array, but rather I should just intercept
// when things happen to it and update the backing git_tree instead.
// This might be hard though, given that libgit2's support for manipulating tree entries is somewhat limited (can't insert entries anywhere but end of list for example).
// One approach I could take is to just git_tree_clear_entries() when the Tree is saved. Seems a waste though.
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

	tree->tree_ = (git_tree*)theTree->Value();
	tree->entryCount_ = git_tree_entrycount(tree->tree_);

	const git_oid *treeOid = git_tree_id(tree->tree_);
	if(treeOid) {
		args.This()->Set(ID_PROPERTY, String::New(git_oid_allocfmt(treeOid)), ReadOnly);
	}
	else {
		args.This()->Set(ID_PROPERTY, Null(), ReadOnly);
	}

	args.This()->Set(LENGTH_PROPERTY, Integer::New(tree->entryCount_), ReadOnly);

	tree->Wrap(args.This());
	return args.This();
}

Handle<Value> Tree::GetEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());
	git_tree_entry *entry;

	if(args[0]->IsString()) {
		REQ_STR_ARG(0, propertyName);
		entry = git_tree_entry_byname(tree->tree_, const_cast<const char*>(*propertyName));
	}
	else {
		REQ_INT_ARG(0, indexArg);
		entry = git_tree_entry_byindex(tree->tree_, indexArg);
	}

	if(entry == NULL) {
		return scope.Close(Null());
	}

	RETURN_WRAP_TREE_ENTRY();
}

Handle<Value> Tree::AddEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(3);
	REQ_OID_ARG(0, idArg);
	REQ_STR_ARG(1, filenameArg);
	REQ_INT_ARG(2, modeArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	git_tree_entry *entry;
	int res = git_tree_add_entry(&entry, tree->tree_, &idArg, *filenameArg, modeArg);
	if(res != GIT_SUCCESS) {
		THROW_GIT_ERROR("Error creating tree entry.", res);
	}

	tree->entryCount_++;
	UPDATE_ENTRY_COUNT();

	RETURN_WRAP_TREE_ENTRY();
}

Handle<Value> Tree::RemoveEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, indexArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());
	int res;

	if(args[0]->IsString()) {
		REQ_STR_ARG(0, propertyName);

		git_tree_entry *entry = git_tree_entry_byname(tree->tree_, *propertyName);
		if(!entry) {
			THROW_ERROR("Entry with that name not found.");
		}

		tree->entryStore_.deleteObjectFor(entry);

		res = git_tree_remove_entry_byname(tree->tree_, *propertyName);
	}
	else {
		REQ_INT_ARG(0, indexArg);

		git_tree_entry *entry = git_tree_entry_byindex(tree->tree_, indexArg);

		if(!entry) {
			THROW_ERROR("Entry with that index not found.");
		}

		// Delete the entry from objectstore.
		tree->entryStore_.deleteObjectFor(entry);

		res = git_tree_remove_entry_byindex(tree->tree_, indexArg);
	}

	if(res != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't delete tree entry", res);
	}

	tree->entryCount_--;
	UPDATE_ENTRY_COUNT();

	return scope.Close(True());
}

Handle<Value> Tree::Clear(const Arguments& args) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	git_tree_clear_entries(tree->tree_);

	tree->entryCount_ = 0;
	UPDATE_ENTRY_COUNT();

	return scope.Close(Undefined());
}

Handle<Value> Tree::Save(const Arguments& args) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	int result = git_object_write((git_object *)tree->tree_);
	if(result != GIT_SUCCESS) {
		return ThrowException(CreateGitError(String::New("Error saving tree."), result));
	}

	const git_oid *treeOid = git_tree_id(tree->tree_);
	args.This()->ForceSet(String::New("id"), String::New(git_oid_allocfmt(treeOid)), ReadOnly);

	return Undefined();
}

TreeEntry *Tree::wrapEntry(git_tree_entry *entry) {
	HandleScope scope;

	TreeEntry *entryObject;
	if(entryStore_.getObjectFor(entry, &entryObject)) {
		entryObject->tree_ = this;
	}

	return entryObject;
}

Tree::~Tree() {
}

} // namespace gitteh
