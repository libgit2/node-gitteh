#include "tree_entry.h"

namespace gitteh {

Persistent<FunctionTemplate> TreeEntry::constructor_template;

void TreeEntry::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("TreeEntry"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> TreeEntry::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theEntry);

	TreeEntry *entry = new TreeEntry();
	entry->entry_ = (git_tree_entry*)theEntry->Value();

	args.This()->Set(String::New("id"), String::New(git_oid_allocfmt(git_tree_entry_id(entry->entry_))));
	args.This()->Set(String::New("attributes"), Integer::New(git_tree_entry_attributes(entry->entry_)));
	args.This()->Set(String::New("filename"), String::New(git_tree_entry_name(entry->entry_)));

	entry->Wrap(args.This());
	return args.This();
}

TreeEntry::~TreeEntry() {
}

} // namespace gitteh
