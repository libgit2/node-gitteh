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
	static Handle<Value> Save(const Arguments&);

	void processInitData(void*);
	void* loadInitData();

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);
};

} // namespace gitteh

#endif	// GITTEH_TREE_H
