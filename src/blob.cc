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

#include <node_buffer.h>
#include "blob.h"

static Persistent<String> blob_class_symbol;
static Persistent<String> id_symbol;
static Persistent<String> data_symbol;

namespace gitteh {

Persistent<FunctionTemplate> Blob::constructor_template;

void Blob::Init(Handle<Object> target) {
	HandleScope scope;

	blob_class_symbol = NODE_PSYMBOL("Blob");
	id_symbol = NODE_PSYMBOL("id");
	data_symbol = NODE_PSYMBOL("data");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(blob_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	target->Set(blob_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Blob::New(const Arguments& args) {
	HandleScope scope;
	REQ_EXT_ARG(0, blobArg);

	Blob *blobObj = static_cast<Blob*>(blobArg->Value());
	blobObj->Wrap(args.This());

	Handle<Object> me = args.This();

	git_blob *blob = blobObj->blob_;

	ImmutableSet(me, id_symbol, CastToJS(&blobObj->oid_));

	int blobSize = git_blob_rawsize(blob);
	Buffer *buffer = Buffer::New((char*)git_blob_rawcontent(blob), blobSize);
	ImmutableSet(me, data_symbol, MakeFastBuffer(buffer, blobSize));

	return args.This();
}

Blob::Blob(git_blob *blob) : GitObject((git_object*)blob) {
	blob_ = blob;
}

Blob::~Blob() {
	if(blob_) {
		git_blob_free(blob_);
		blob_ = NULL;
	}
}

}; // namespace gitteh
