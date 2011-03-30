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
#include "repository.h"
#include "index.h"
#include "index_entry.h"
#include "tag.h"
#include "rev_walker.h"
#include "error.h"
#include "ref.h"
#include "blob.h"
#include "thread.h"

namespace gitteh {

#if 0
static void gcNotif(GCType type, GCCallbackFlags flags) {
	std::cout << "gc is happening yo.\n";
}
#endif

Persistent<FunctionTemplate> construct;
class Feck : public ObjectWrap {
public:
	gitteh_lock myLock;

	static int EIO_Unlock(eio_req *req) {
		Feck *feck = static_cast<Feck*>(req->data);

		std::cout << "eio start!\n";
		sleep(2);
		std::cout << "done sleepin'\n";
		UNLOCK_MUTEX(feck->myLock);

		return 0;
	}

	static int EIO_AfterUnlock(eio_req *req) {
		std::cout << "afterunlock\n";
		Feck *feck = static_cast<Feck*>(req->data);
		feck->Unref();
		ev_unref(EV_DEFAULT_UC);
		return 0;
	}

	static Handle<Value> AsyncLock(const Arguments& args) {
		HandleScope scope;
		Feck *feck = ObjectWrap::Unwrap<Feck>(args.This());

		LOCK_MUTEX(feck->myLock);
		feck->Ref();

		eio_custom(EIO_Unlock, EIO_PRI_DEFAULT, EIO_AfterUnlock, feck);
		ev_ref(EV_DEFAULT_UC);
		std::cout << "asynclock.\n";

		return Undefined();
	}

	static Handle<Value> SyncLock(const Arguments& args) {
		HandleScope scope;
		Feck *feck = ObjectWrap::Unwrap<Feck>(args.This());

		LOCK_MUTEX(feck->myLock);
		UNLOCK_MUTEX(feck->myLock);
		std::cout << "synclock done.\n";
		return Undefined();
	}

	Feck() {
		CREATE_MUTEX(myLock);
	}

	static Handle<Value> New(const Arguments& args) {
		Feck *feck = new Feck();
		feck->Wrap(args.This());

		return args.This();
	}

	static void Init(Handle<Object> target) {
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		t->InstanceTemplate()->SetInternalFieldCount(1);
		construct = Persistent<FunctionTemplate>::New(t);

		NODE_SET_PROTOTYPE_METHOD(t, "syncLock", SyncLock);
		NODE_SET_PROTOTYPE_METHOD(t, "asyncLock", AsyncLock);
	}
};



extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Repository::Init(target);

	Commit::Init(target);
	Tree::Init(target);
	Index::Init(target);
	IndexEntry::Init(target);
	Tag::Init(target);
	RevWalker::Init(target);
	Reference::Init(target);
	Blob::Init(target);

	ErrorInit(target);

	//V8::AddGCPrologueCallback(gcNotif);
	Feck::Init(target);
	target->Set(String::New("feck"), construct->GetFunction());
}

} // namespace gitteh
