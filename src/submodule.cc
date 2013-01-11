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

#include "submodule.h"

static Persistent<String> name_symbol;
static Persistent<String> path_symbol;
static Persistent<String> url_symbol;
static Persistent<String> oid_symbol;

namespace gitteh {
	namespace Submodule {
		void Init(Handle<Object> target) {
			HandleScope scope;
			name_symbol = NODE_PSYMBOL("name");
			path_symbol	= NODE_PSYMBOL("path");
			url_symbol 	= NODE_PSYMBOL("url");
			oid_symbol 	= NODE_PSYMBOL("oid");
		}

		Handle<Object> Create(git_submodule *submodule) {
			HandleScope scope;
			Handle<Object> o = Object::New();

			o->Set(name_symbol, CastToJS(submodule->name));
			o->Set(path_symbol, CastToJS(submodule->path));
			o->Set(url_symbol, CastToJS(submodule->url));
			o->Set(oid_symbol, CastToJS(submodule->oid));
			return scope.Close(o);
		}
	}
};
