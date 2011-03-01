#include "tree.h"
#include "tree_entry.h"

Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTree);

	Tree *tree = new Tree();
	tree->tree_ = (git_tree*)theTree->Value();
	tree->entryCount_ = git_tree_entrycount(tree->tree_);

	args.This()->Set(String::New("id"), String::New(git_oid_allocfmt(git_tree_id(tree->tree_))), ReadOnly);

	Handle<ObjectTemplate> entriesObjectTemplate = ObjectTemplate::New();
	entriesObjectTemplate->SetInternalFieldCount(1);
	entriesObjectTemplate->SetIndexedPropertyHandler(EntryIndexedHandler);
	//entriesObjectTemplate->SetNamedPropertyHandler(EntryNamedHandler);

	Handle<Object> entriesObject = entriesObjectTemplate->NewInstance();
	entriesObject->SetInternalField(0, args.This());
	args.This()->Set(String::New("entries"), entriesObject);
	entriesObject->Set(String::New("length"), Persistent<Integer>::New(Integer::New(tree->entryCount_)));

	tree->Wrap(args.This());
	return args.This();
}

Handle<Value> Tree::EntryIndexedHandler(uint32_t index, const AccessorInfo& info) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(Local<Object>::Cast(info.This()->GetInternalField(0)));

	if(index >= tree->entryCount_) {
		return ThrowException(Exception::Error(String::New("Tree entry index is out of range.")));
	}

	git_tree_entry *entry = git_tree_entry_byindex(tree->tree_, index);

	TreeEntry *treeEntryObject = tree->wrapEntry(entry);
	return scope.Close(treeEntryObject->handle_);
}

Handle<Value> Tree::EntryNamedHandler(Local<String> propertyName, const AccessorInfo& info) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(Local<Object>::Cast(info.This()->GetInternalField(0)));
	git_tree_entry *entry = git_tree_entry_byname(tree->tree_, const_cast<const char*>(*String::Utf8Value(propertyName)));

	if(entry == NULL) {
		return scope.Close(Null());
	}

	TreeEntry *treeEntryObject = tree->wrapEntry(entry);
	return scope.Close(treeEntryObject->handle_);
}

TreeEntry *Tree::wrapEntry(git_tree_entry *entry) {
	HandleScope scope;

	TreeEntry *entryObject;
	if(entryStore_.getObjectFor(entry, &entryObject)) {
		entryObject->tree_ = this;
	}

	return entryObject;
}

Tree::~Tree() {
}
