#include "index.h"

Persistent<FunctionTemplate> Index::constructor_template;

void Index::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Index"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> Index::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theIndex);

	Index *index = new Index();
	index->index_ = (git_index*)theIndex->Value();

	git_index_read(index->index_);
	index->entryCount_ = git_index_entrycount(index->index_);

	Handle<ObjectTemplate> entriesTemplate = ObjectTemplate::New();
	entriesTemplate->SetInternalFieldCount(1);
	entriesTemplate->SetIndexedPropertyHandler(EntriesGetter);

	Handle<Object> entriesObject = entriesTemplate->NewInstance();
	entriesObject->SetInternalField(0, args.This());
	entriesObject->Set(String::New("length"), Integer::New(index->entryCount_));

	args.This()->Set(String::New("entries"), entriesObject);
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
