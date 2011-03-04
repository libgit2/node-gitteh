#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "object_store.h"

namespace gitteh {

class TreeEntry;
class Repository;

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

class Tree : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	~Tree();

	TreeEntry *wrapEntry(git_tree_entry*);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> GetEntry(const Arguments&);
	static Handle<Value> AddEntry(const Arguments&);
	static Handle<Value> RemoveEntry(const Arguments&);
	static Handle<Value> Clear(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	git_tree *tree_;
	size_t entryCount_;
	ObjectStore<TreeEntry, git_tree_entry> entryStore_;
};

} // namespace gitteh

#endif	// GITTEH_TREE_H
