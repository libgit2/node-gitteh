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

static Persistent<String> id_symbol;
static Persistent<String> entries_symbol;
static Persistent<String> entry_id_symbol;
static Persistent<String> entry_name_symbol;
static Persistent<String> entry_type_symbol;
static Persistent<String> entry_attributes_symbol;

static Handle<Object> CreateEntry(const git_tree_entry *entry) {
	HandleScope scope;
	Handle<Object> o = Object::New();
	o->Set(entry_id_symbol, CastToJS(git_tree_entry_id(entry)));
	o->Set(entry_name_symbol, CastToJS(git_tree_entry_name(entry)));
	o->Set(entry_attributes_symbol, CastToJS(git_tree_entry_attributes(entry)));
	o->Set(entry_type_symbol, CastToJS(git_tree_entry_type(entry)));
	return scope.Close(o);
}

namespace gitteh {
	namespace Tree {
		void Init(Handle<Object> target) {
			HandleScope scope;
			id_symbol 				= NODE_PSYMBOL("id");
			entries_symbol 			= NODE_PSYMBOL("entries");
			entry_id_symbol 		= NODE_PSYMBOL("id");
			entry_name_symbol 		= NODE_PSYMBOL("name");
			entry_type_symbol 		= NODE_PSYMBOL("type");
			entry_attributes_symbol = NODE_PSYMBOL("attributes");
		}

		Handle<Object> Create(git_tree *tree) {
			HandleScope scope;
			Handle<Object> o = Object::New();

			Handle<Array> entries = Array::New();
			int entryCount = git_tree_entrycount(tree);
			for(int i = 0; i < entryCount; i++) {
				entries->Set(i, CreateEntry(git_tree_entry_byindex(tree, i)));
			}
			o->Set(entries_symbol, entries);

			return scope.Close(o);
		}
	};
};
