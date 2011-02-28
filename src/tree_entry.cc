#include "tree_entry.h"

Persistent<FunctionTemplate> TreeEntry::constructor_template;

void TreeEntry::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("TreeEntry"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	t->PrototypeTemplate()->SetAccessor(TREE_ENTRY_NAME_SYMBOL, NameGetter);
}

Handle<Value> TreeEntry::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theEntry);

	TreeEntry *entry = new TreeEntry();
	entry->entry_ = (git_tree_entry*)theEntry->Value();

	entry->Wrap(args.This());
	entry->MakeWeak();

	return args.This();
}

Handle<Value> TreeEntry::NameGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	TreeEntry *entry = ObjectWrap::Unwrap<TreeEntry>(info.This());
	const char* fileName = git_tree_entry_name(entry->entry_);

	return scope.Close(String::New(fileName));
}
