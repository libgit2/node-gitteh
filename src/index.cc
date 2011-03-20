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

#include "index.h"
#include "index_entry.h"
#include "repository.h"

#define LENGTH_PROPERTY String::NewSymbol("entryCount")

namespace gitteh {

struct index_data {
	int entryCount;
};


Persistent<FunctionTemplate> Index::constructor_template;

Index::~Index() {
	repository_->notifyIndexDead();
	git_index_free(index_);
}

void Index::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Index"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> Index::New(const Arguments& args) {
	HandleScope scope;

	//REQ_ARGS(1);
	//REQ_EXT_ARG(0, theIndex);

	Index *index = new Index();
	index->Wrap(args.This());

	return args.This();
}

Handle<Value> Index::EntriesGetter(uint32_t i, const AccessorInfo& info) {
	HandleScope scope;

	Index *index = ObjectWrap::Unwrap<Index>(Local<Object>::Cast(info.This()->GetInternalField(0)));

	if(i >= index->entryCount_) {
		return ThrowException(Exception::Error(String::New("Index index out of bounds.")));
	}

	git_index_entry *entry = git_index_get(index->index_, i);

	IndexEntry *entryObject;
	if(index->entryStore_.getObjectFor(entry, &entryObject)) {

	}

	return scope.Close(entryObject->handle_);
}

void Index::processInitData(void *data) {
	if(data != NULL) {
		index_data *indexData = static_cast<index_data*>(data);

		entryCount_ = indexData->entryCount;
		handle_->Set(LENGTH_PROPERTY, Integer::New(indexData->entryCount),
				(PropertyAttribute)(ReadOnly | DontDelete));

		delete indexData;
	}
}

void *Index::loadInitData() {
	repository_->lockRepository();
	initError_ = git_repository_index(&index_, repository_->repo_);
	repository_->unlockRepository();
	if(initError_ == GIT_EBAREINDEX) {
		repository_->lockRepository();
		initError_ = git_index_open_bare(&index_, repository_->path_);
		repository_->unlockRepository();
	}

	if(initError_ != GIT_SUCCESS) {
		return NULL;
	}

	index_data *data = new index_data;

	repository_->lockRepository();
	git_index_read(index_);
	data->entryCount = git_index_entrycount(index_);
	repository_->unlockRepository();

	return data;
}

void Index::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
