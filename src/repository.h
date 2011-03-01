#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"
#include "object_store.h"

class Commit;
class Tree;
class Index;

class Repository : public ObjectWrap {
public:
	~Repository();
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Tree *wrapTree(git_tree *tree);
	Commit *wrapCommit(git_commit *commit);

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetODB(const Arguments&);
	static Handle<Value> GetCommit(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> IndexGetter(Local<String>, const AccessorInfo&);

	void close();

	git_repository *repo_;
	ObjectStore<Commit, git_commit> commitStore_;
	ObjectStore<Tree, git_tree> treeStore_;
	Index *index_;
};

#endif // GITTEH_REPO_H
