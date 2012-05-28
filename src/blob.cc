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

#include "blob.h"
#include <node_buffer.h>

static Persistent<String> id_symbol;
static Persistent<String> data_symbol;

namespace gitteh {
	namespace Blob {
		void Init(Handle<Object> target) {
			HandleScope scope;
			id_symbol = 	NODE_PSYMBOL("id");
			data_symbol = 	NODE_PSYMBOL("data");
		}

		Handle<Value> Create(git_blob *blob) {
			HandleScope scope;
			Handle<Object> o = Object::New();
			o->Set(id_symbol, CastToJS(git_object_id((git_object*)blob)));
			int len = git_blob_rawsize(blob);
			Buffer *buffer = Buffer::New((char*)git_blob_rawcontent(blob), len);
			o->Set(data_symbol, MakeFastBuffer(buffer, len));
			return scope.Close(o);
		}
	}
};
