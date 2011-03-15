#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "object_store.h"
#include "ts_objectwrap.h"

namespace gitteh {

class TreeEntry;
class Repository;

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

class Tree : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	~Tree();

	TreeEntry *wrapEntry(git_tree_entry*);

	void setOwner(void*);

	git_tree *tree_;
	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> GetEntry(const Arguments&);
	static Handle<Value> AddEntry(const Arguments&);
	static Handle<Value> RemoveEntry(const Arguments&);
	static Handle<Value> Clear(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData(void *data);
	void* loadInitData();

	size_t entryCount_;
	ObjectStore<TreeEntry, git_tree_entry> entryStore_;
};

} // namespace gitteh

#endif	// GITTEH_TREE_H
