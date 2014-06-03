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

#include "gitteh.h"
#include "repository.h"
#include "commit.h"
#include "signature.h"
#include "tree.h"
#include "blob.h"
#include "tag.h"
#include "remote.h"
#include "index.h"

namespace gitteh {

Persistent<Object> module;

static Handle<Object> CreateTypeObject() {
	NanEscapableScope();
	Handle<Object> o = NanNew<Object>();
	ImmutableSet(o, NanNew<String>("commit"), NanNew<Number>(GIT_OBJ_COMMIT));
	ImmutableSet(o, NanNew<String>("tree"), NanNew<Number>(GIT_OBJ_TREE));
	ImmutableSet(o, NanNew<String>("blob"), NanNew<Number>(GIT_OBJ_BLOB));
	ImmutableSet(o, NanNew<String>("tag"), NanNew<Number>(GIT_OBJ_TAG));
	return NanEscapeScope(o);
}

extern "C" void
init(Handle<Object> target) {
	NanScope();
  module = Persistent<Object>::New(target);

	// Initialize libgit2's thread system.
	git_threads_init();

	Signature::Init();
	Repository::Init(target);
	Commit::Init(target);
	Tree::Init(target);
	Blob::Init(target);
	Tag::Init(target);
	Index::Init(target);

	Remote::Init(target);

	ImmutableSet(target, NanNew<String>("minOidLength"), NanNew<Number>(GIT_OID_MINPREFIXLEN));
	ImmutableSet(target, NanNew<String>("types"), CreateTypeObject());

	NODE_DEFINE_CONSTANT(target, GIT_DIRECTION_PUSH);
	NODE_DEFINE_CONSTANT(target, GIT_DIRECTION_FETCH);

	/*
	IndexEntry::Init(target);

	RevWalker::Init(target);
	Reference::Init(target);

	ErrorInit(target);*/
}

Handle<Object> GetModule() {
	return module;
}

} // namespace gitteh

NODE_MODULE(gitteh, gitteh::init)
