#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

class TreeEntry;
class Repository;

struct tree_data;

class Tree : public WrappedGitObject<Tree, git_tree> {
public:
	Tree(git_tree*);
	~Tree();

	void setOwner(void*);

	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	git_tree *tree_;
	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData();
	int doInit();

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);

	tree_data *initData_;
};

} // namespace gitteh

#endif	// GITTEH_TREE_H
