#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "gitobjectwrap.h"
#include "object_factory.h"

namespace gitteh {

class TreeEntry;
class Repository;

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

template <class, class, class> class ObjectFactory;

class Tree : public GitObjectWrap {
public:
	template<class, class,class> friend class ObjectFactory;

	Tree();
	~Tree();

	void setOwner(void*);

	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	git_tree *tree_;
	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> GetEntry(const Arguments&);
	static Handle<Value> AddEntry(const Arguments&);
	static Handle<Value> RemoveEntry(const Arguments&);
	static Handle<Value> Clear(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData(void*);
	void* loadInitData();

	size_t entryCount_;
	ObjectFactory<Tree, TreeEntry, git_tree_entry> *entryFactory_;
private:
	static int EIO_GetEntry(eio_req*);
	static int EIO_AfterGetEntry(eio_req*);

	static int EIO_AddEntry(eio_req*);
	static int EIO_AfterAddEntry(eio_req*);

	static int EIO_RemoveEntry(eio_req*);
	static int EIO_AfterRemoveEntry(eio_req*);

	static int EIO_ClearEntries(eio_req*);
	static int EIO_AfterClearEntries(eio_req*);

	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);
};

} // namespace gitteh

#endif	// GITTEH_TREE_H
