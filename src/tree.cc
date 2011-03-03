#include "tree.h"
#include "tree_entry.h"

namespace gitteh {

Persistent<FunctionTemplate> Tree::constructor_template;

void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getByName", GetByName);

	NODE_SET_PROTOTYPE_METHOD(t, "addEntry", AddEntry);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTree);

	Tree *tree = new Tree();
	tree->tree_ = (git_tree*)theTree->Value();
	tree->entryCount_ = git_tree_entrycount(tree->tree_);

	Handle<Array> entriesArray = Array::New(tree->entryCount_);

	const git_oid *treeOid = git_tree_id(tree->tree_);
	if(treeOid) {
		args.This()->Set(String::New("id"), String::New(git_oid_allocfmt(treeOid)), ReadOnly);

		git_tree_entry *entry;
		TreeEntry *treeEntryObject;
		for(int i = 0; i < tree->entryCount_; i++) {
			entry = git_tree_entry_byindex(tree->tree_, i);
			treeEntryObject = tree->wrapEntry(entry);
			entriesArray->Set(i, Local<Object>::New(treeEntryObject->handle_));
		}
	}
	else {
		args.This()->Set(String::New("id"), Null(), ReadOnly);
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

Handle<Value> Tree::AddEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(3);
	REQ_OID_ARG(0, idArg);
	REQ_STR_ARG(1, filenameArg);
	REQ_INT_ARG(2, modeArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	git_tree_entry *entry;
	int res = git_tree_add_entry(&entry, tree->tree_, &idArg, *filenameArg, modeArg);
	if(res != GIT_SUCCESS) {
		return ThrowException(Exception::Error(String::New("Error creating tree entry.")));
	}

	TreeEntry *treeEntryObject;
	treeEntryObject = tree->wrapEntry(entry);
	Local<Array>::Cast(args.This()->Get(String::New("entries")))->Set(tree->entryCount_++, Local<Object>::New(treeEntryObject->handle_));

	return Undefined();
}

Handle<Value> Tree::Save(const Arguments& args) {
	HandleScope scope;

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());

	int result = git_object_write((git_object *)tree->tree_);
	if(result != GIT_SUCCESS) {
		return ThrowException(ThrowGitError(String::New("Error saving tree."), result));
	}

	const git_oid *treeOid = git_tree_id(tree->tree_);
	args.This()->ForceSet(String::New("id"), String::New(git_oid_allocfmt(treeOid)), ReadOnly);

	return Undefined();
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

} // namespace gitteh
