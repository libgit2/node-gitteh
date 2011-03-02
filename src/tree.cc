#include "tree.h"
#include "tree_entry.h"

Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getByName", GetByName);
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTree);

	Tree *tree = new Tree();
	tree->tree_ = (git_tree*)theTree->Value();
	tree->entryCount_ = git_tree_entrycount(tree->tree_);

	args.This()->Set(String::New("id"), String::New(git_oid_allocfmt(git_tree_id(tree->tree_))), ReadOnly);

	/*Handle<ObjectTemplate> entriesObjectTemplate = ObjectTemplate::New();
	entriesObjectTemplate->SetInternalFieldCount(1);
	//entriesObjectTemplate->SetIndexedPropertyHandler(EntryIndexedHandler);
	//entriesObjectTemplate->SetNamedPropertyHandler(EntryNamedHandler);

	Handle<Object> entriesObject = entriesObjectTemplate->NewInstance();
	entriesObject->SetInternalField(0, args.This());
	args.This()->Set(String::New("entries"), entriesObject);
	entriesObject->Set(String::New("length"), Persistent<Integer>::New(Integer::New(tree->entryCount_)));*/

	Handle<Array> entriesArray = Array::New(tree->entryCount_);

	git_tree_entry *entry;
	TreeEntry *treeEntryObject;
	for(int i = 0; i < tree->entryCount_; i++) {
		entry = git_tree_entry_byindex(tree->tree_, i);
		treeEntryObject = tree->wrapEntry(entry);
		entriesArray->Set(i, Local<Object>::New(treeEntryObject->handle_));
	}

	args.This()->Set(String::New("entries"), entriesArray);

	tree->Wrap(args.This());
	return args.This();
}

Handle<Value> Tree::GetByName(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_STR_ARG(0, propertyName);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());
	git_tree_entry *entry = git_tree_entry_byname(tree->tree_, const_cast<const char*>(*propertyName));

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
