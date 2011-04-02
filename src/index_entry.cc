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

#include "index_entry.h"

static Persistent<String> entry_class_symbol;

#define CTIME_PROPERTY String::NewSymbol("ctime")
#define MTIME_PROPERTY String::NewSymbol("mtime")
#define DEV_PROPERTY String::NewSymbol("dev")
#define INO_PROPERTY String::NewSymbol("ino")
#define MODE_PROPERTY String::NewSymbol("mode")
#define UID_PROPERTY String::NewSymbol("uid")
#define GID_PROPERTY String::NewSymbol("gid")
#define SIZE_PROPERTY String::NewSymbol("file_size")
#define OID_PROPERTY String::NewSymbol("oid")
#define FLAGS_PROPERTY String::NewSymbol("flags")
#define FLAGS_EXTENDED_PROPERTY String::NewSymbol("flags_extended")
#define PATH_PROPERTY String::NewSymbol("path")

namespace gitteh {

Persistent<FunctionTemplate> IndexEntry::constructor_template;

void IndexEntry::Init(Handle<Object> target) {
	HandleScope scope;

	entry_class_symbol = NODE_PSYMBOL("IndexEntry");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(entry_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	target->Set(entry_class_symbol, constructor_template->GetFunction());
}

Handle<Value> IndexEntry::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theEntry);

	IndexEntry *entry = new IndexEntry();
	entry->entry_ = (git_index_entry*)theEntry->Value();

	entry->Wrap(args.This());
	return args.This();
}

struct dummy {

};

void *IndexEntry::loadInitData() {
	// We don't need to bother loading data here, since everything is already
	// loaded into a struct for us. However we need to return something.
	return new dummy;
}

void IndexEntry::processInitData(void *data) {
	delete static_cast<dummy*>(data);

	handle_->Set(CTIME_PROPERTY, Date::New((double)entry_->ctime.seconds*1000));
	handle_->Set(MTIME_PROPERTY, Date::New((double)entry_->mtime.seconds*1000));
	handle_->Set(DEV_PROPERTY, Integer::New(entry_->dev));
	handle_->Set(INO_PROPERTY, Integer::New(entry_->ino));
	handle_->Set(MODE_PROPERTY, Integer::New(entry_->mode));
	handle_->Set(UID_PROPERTY, Integer::New(entry_->uid));
	handle_->Set(GID_PROPERTY, Integer::New(entry_->gid));
	handle_->Set(SIZE_PROPERTY, Integer::New(entry_->file_size));
	char oidStr[40];
	git_oid_fmt(oidStr, &entry_->oid);
	handle_->Set(OID_PROPERTY, String::New(oidStr, 40));
	handle_->Set(FLAGS_PROPERTY, Integer::New(entry_->flags));
	handle_->Set(FLAGS_EXTENDED_PROPERTY, Integer::New(entry_->flags_extended));
	handle_->Set(PATH_PROPERTY, String::New(entry_->path));
}

void IndexEntry::setOwner(void *owner) {
	index_ = static_cast<Index*>(owner);
}

} // namespace gitteh
