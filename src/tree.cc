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

static Persistent<String> tree_class_symbol;
static Persistent<String> id_symbol;
static Persistent<String> entries_symbol;
static Persistent<String> entry_id_symbol;
static Persistent<String> entry_name_symbol;
static Persistent<String> entry_type_symbol;
static Persistent<String> entry_attributes_symbol;

namespace gitteh {

Persistent<FunctionTemplate> Tree::constructor_template;

static Handle<Object> CreateEntryObject(const git_tree_entry *entry) {
	HandleScope scope;

	Handle<Object> obj = Object::New();

	ImmutableSet(obj, entry_id_symbol, CastToJS(git_tree_entry_id(entry)));
	ImmutableSet(obj, entry_name_symbol, CastToJS(git_tree_entry_name(entry)));
	ImmutableSet(obj, entry_attributes_symbol, CastToJS(
		git_tree_entry_attributes(entry)));
	ImmutableSet(obj, entry_type_symbol, CastToJS(GitObjectTypeToString(
		git_tree_entry_type(entry))));
	return scope.Close(obj);
}

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	tree_class_symbol 		= NODE_PSYMBOL("Tree");
	id_symbol 				= NODE_PSYMBOL("id");
	entries_symbol 			= NODE_PSYMBOL("entries");
	entry_id_symbol 		= NODE_PSYMBOL("id");
	entry_name_symbol 		= NODE_PSYMBOL("name");
	entry_type_symbol 		= NODE_PSYMBOL("type");
	entry_attributes_symbol = NODE_PSYMBOL("attributes");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(tree_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	target->Set(tree_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;
	REQ_EXT_ARG(0, treeArg);

	Tree *treeObj = static_cast<Tree*>(treeArg->Value());
	treeObj->Wrap(args.This());

	Handle<Object> me = args.This();

	git_tree *tree = treeObj->tree_;

	ImmutableSet(me, id_symbol, CastToJS(&treeObj->oid_));

	Handle<Array> entries = Array::New();
	int entryCount = git_tree_entrycount(tree);
	for(int i = 0; i < entryCount; i++) {
		entries->Set(i, CreateEntryObject(git_tree_entry_byindex(tree, i)));
	}
	ImmutableSet(me, entries_symbol, entries);

	return args.This();
}

Tree::Tree(git_tree *tree) : GitObject((git_object*)tree) {
	tree_ = tree;
}

Tree::~Tree() {
	std::cout << "Tree dying." << std::endl;
	if(tree_) {
		git_tree_free(tree_);
		tree_ = NULL;
	}
}

} // namespace gitteh
