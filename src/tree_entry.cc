#include "tree_entry.h"

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

	REQ_ARGS(2);
	REQ_EXT_ARG(0, theEntry);
	REQ_EXT_ARG(0, theTree);

	TreeEntry *entry = new TreeEntry();
	entry->entry_ = (git_tree_entry*)theEntry->Value();
	entry->tree_ = (Tree*)theTree->Value();

	entry->Wrap(args.This());
	entry->MakeWeak();

	args.This()->Set(String::New("id"), String::New(git_oid_allocfmt(git_tree_entry_id(entry->entry_))));
	args.This()->Set(String::New("attributes"), Integer::New(git_tree_entry_attributes(entry->entry_)));
	args.This()->Set(String::New("filename"), String::New(git_tree_entry_name(entry->entry_)));

	return args.This();
}
