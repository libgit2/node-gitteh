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
#include "signature.h"

static Persistent<String> message_symbol;
static Persistent<String> message_encoding_symbol;
static Persistent<String> author_symbol;
static Persistent<String> committer_symbol;
static Persistent<String> tree_symbol;
static Persistent<String> parents_symbol;

namespace gitteh {
	namespace Commit {
		void Init(Handle<Object> target) {
			HandleScope scope;
			message_symbol = 			NODE_PSYMBOL("message");
			message_encoding_symbol = 	NODE_PSYMBOL("messageEncoding");
			author_symbol = 			NODE_PSYMBOL("author");
			committer_symbol = 			NODE_PSYMBOL("committer");
			tree_symbol = 				NODE_PSYMBOL("tree");
			parents_symbol = 			NODE_PSYMBOL("parents");
		}

		Handle<Object> Create(git_commit *cm) {
			HandleScope scope;
			Handle<Object> o = Object::New();
			o->Set(tree_symbol, CastToJS(git_commit_tree_oid(cm)));
			o->Set(message_symbol, CastToJS(git_commit_message(cm)));
			const char *encoding = git_commit_message_encoding(cm);
			if(encoding) {
				o->Set(message_encoding_symbol, CastToJS(encoding));
			}
			Handle<Array> parents = Array::New();
			int parentCount = git_commit_parentcount(cm);
			for(int i = 0; i < parentCount; i++) {
				parents->Set(i, CastToJS(git_commit_parent_oid(cm, i)));
			}
			o->Set(parents_symbol, parents);
			o->Set(author_symbol, CastToJS(git_commit_author(cm)));
			o->Set(committer_symbol, CastToJS(git_commit_committer(cm)));

			return scope.Close(o);
		}
	};
};
