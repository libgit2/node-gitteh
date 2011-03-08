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

#include "commit.h"
#include "tree.h"
#include "tree_entry.h"
#include "rawobj.h"
#include "repository.h"
#include "index.h"
#include "index_entry.h"
#include "tag.h"
#include "rev_walker.h"
#include "error.h"
#include "ref.h"

namespace gitteh {

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Repository::Init(target);
	RawObject::Init(target);
	Commit::Init(target);
	Tree::Init(target);
	TreeEntry::Init(target);
	Index::Init(target);
	IndexEntry::Init(target);
	Tag::Init(target);
	RevWalker::Init(target);
	Reference::Init(target);

	ErrorInit(target);
}

} // namespace gitteh
