#include "index_entry.h"

namespace gitteh {

Persistent<FunctionTemplate> IndexEntry::constructor_template;

void IndexEntry::Init(Handle<Object>) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("IndexEntry"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> IndexEntry::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theEntry);

	IndexEntry *entry = new IndexEntry();
	entry->entry_ = (git_index_entry*)theEntry->Value();

	args.This()->Set(String::New("path"), String::New(entry->entry_->path));

	entry->Wrap(args.This());
	return args.This();
}

} // namespace gitteh
