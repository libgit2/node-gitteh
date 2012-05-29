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

#include "tag.h"

static Persistent<String> name_symbol;
static Persistent<String> message_symbol;
static Persistent<String> tagger_symbol;
static Persistent<String> target_symbol;
static Persistent<String> type_symbol;

namespace gitteh {
	namespace Tag {
		void Init(Handle<Object> target) {
			HandleScope scope;
			name_symbol 	= NODE_PSYMBOL("name");
			message_symbol	= NODE_PSYMBOL("message");
			tagger_symbol 	= NODE_PSYMBOL("tagger");
			target_symbol 	= NODE_PSYMBOL("target");
			type_symbol 	= NODE_PSYMBOL("type");
		}

		Handle<Object> Create(git_tag *tag) {
			HandleScope scope;
			Handle<Object> o = Object::New();
			o->Set(name_symbol, CastToJS(git_tag_name(tag)));
			o->Set(message_symbol, CastToJS(git_tag_message(tag)));
			o->Set(tagger_symbol, CastToJS(git_tag_tagger(tag)));
			o->Set(target_symbol, CastToJS(git_tag_target_oid(tag)));
			o->Set(type_symbol, CastToJS(git_tag_type(tag)));
			return scope.Close(o);
		}
	}
};
