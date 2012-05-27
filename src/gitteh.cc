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
/*#include "index.h"
#include "index_entry.h"
#include "tag.h"
#include "rev_walker.h"
#include "error.h"
#include "ref.h"
#include "thread.h"*/

namespace gitteh {

Persistent<Object> module;

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	module = Persistent<Object>::New(target);

	// Initialize libgit2's thread system.
	git_threads_init();

	SignatureInit();
	Repository::Init(target);
	Commit::Init(target);
	Tree::Init(target);
	Blob::Init(target);
	
	/*Index::Init(target);
	IndexEntry::Init(target);
	Tag::Init(target);
	RevWalker::Init(target);
	Reference::Init(target);

	ErrorInit(target);*/
}

Handle<Object> GetModule() {
	return module;
}

} // namespace gitteh
