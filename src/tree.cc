#include "tree.h"
#include "tree_entry.h"

#define LENGTH_PROPERTY String::NewSymbol("length")

namespace gitteh {

Persistent<FunctionTemplate> Tree::constructor_template;

Persistent<ObjectTemplate> entriesWrapperTemplate;

// TODO: I think I'm going about this all wrong. I'm trying to lock down the tree entries array, but rather I should just intercept
// when things happen to it and update the backing git_tree instead.
// This might be hard though, given that libgit2's support for manipulating tree entries is somewhat limited (can't insert entries anywhere but end of list for example).
// One approach I could take is to just git_tree_clear_entries() when the Tree is saved. Seems a waste though.
void Tree::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Tree"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getByName", GetByName);

	NODE_SET_PROTOTYPE_METHOD(t, "addEntry", AddEntry);
	NODE_SET_PROTOTYPE_METHOD(t, "removeEntry", RemoveEntry);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	entriesWrapperTemplate = Persistent<ObjectTemplate>::New(ObjectTemplate::New());
	entriesWrapperTemplate->SetInternalFieldCount(1);
	entriesWrapperTemplate->SetIndexedPropertyHandler(0, SetEntryHandler, 0, DeleteEntryHandler);
	entriesWrapperTemplate->SetNamedPropertyHandler(NamedPropertyGetter, 0, NamedPropertyQuery);
}

Handle<Value> Tree::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTree);

	Tree *tree = new Tree();
	tree->tree_ = (git_tree*)theTree->Value();
	tree->entryCount_ = git_tree_entrycount(tree->tree_);

	Handle<Object> entriesArray = entriesWrapperTemplate->NewInstance();
	entriesArray->SetPointerInInternalField(0, tree);
	entriesArray->SetPrototype(Array::New()->GetPrototype());
	Handle<Object>::Cast(entriesArray->GetPrototype())->ForceDelete(String::New("length"));

	args.This()->Set(String::New("entries"), entriesArray, ReadOnly);

	tree->unlock_ = true;

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

	tree->unlock_ = false;

	args.This()->Set(String::New("entries"), entriesArray, ReadOnly);

	tree->Wrap(args.This());
	return args.This();
}

Handle<Boolean> Tree::DeleteEntryHandler(uint32_t index, const AccessorInfo &info) {
	HandleScope scope;

	Tree *tree = static_cast<Tree*>(info.This()->GetPointerFromInternalField(0));

	if(tree->unlock_ == true) {
		// This basically allows the set through.
		std::cout << "kk.\n";
		//info.Holder()->ForceDelete(Integer::New(index));
		//return scope.Close(Boolean::New(true));
		return scope.Close(Handle<Boolean>());
	}

	// This doesn't.
	return scope.Close(Boolean::New(false));
}

Handle<Value> Tree::SetEntryHandler(uint32_t index, Local< Value > value, const AccessorInfo &info) {
	HandleScope scope;

	Tree *tree = static_cast<Tree*>(info.This()->GetPointerFromInternalField(0));

	if(tree->unlock_ == true) {
		// This basically allows the set through.
		return scope.Close(Handle<Value>());
	}

	// This doesn't.
	return scope.Close(value);
}

Handle<Value> Tree::NamedPropertyGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	if(property == LENGTH_PROPERTY) {
		Tree *tree = static_cast<Tree*>(info.This()->GetPointerFromInternalField(0));

		return scope.Close(Integer::New(tree->entryCount_));
	}

	return scope.Close(Handle<Value>());
}

Handle<Integer> Tree::NamedPropertyQuery(Local<String> property, const AccessorInfo&) {
	HandleScope scope;

	std::cout << "....\n";
	if(property->Equals(LENGTH_PROPERTY)) {
		std::cout << "....\n";
		return scope.Close(Integer::New(DontEnum));
	}

	return scope.Close(Handle<Integer>());
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
		THROW_GIT_ERROR("Error creating tree entry.", res);
	}

	TreeEntry *treeEntryObject;
	treeEntryObject = tree->wrapEntry(entry);

	tree->unlock_ = true;
	Local<Object>::Cast(args.This()->Get(String::New("entries")))->Set(tree->entryCount_++, Local<Object>::New(treeEntryObject->handle_));
	tree->unlock_ = false;

	return Undefined();
}

Handle<Value> Tree::RemoveEntry(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, indexArg);

	Tree *tree = ObjectWrap::Unwrap<Tree>(args.This());
	if(indexArg >= tree->entryCount_) {
		return ThrowException(Exception::Error(String::New("Index out of bounds.")));
	}

	int res = git_tree_remove_entry_byindex(tree->tree_, indexArg);
	if(res != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't delete tree entry", res);
	}

	tree->unlock_ = true;
	//Handle<Object>::Cast(args.This()->Get(String::New("entries")))->Delete(indexArg);
	Handle<Object> entriesObject = Handle<Object>::Cast(args.This()->Get(String::New("entries")));
	Handle<Function> spliceFn = Handle<Function>::Cast(entriesObject->Get(String::New("splice")));
	Handle<Value> spliceArgs[2] = { Integer::New(indexArg), Integer::New(1) };
	spliceFn->Call(entriesObject, 2, spliceArgs);
	tree->unlock_ = false;

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
