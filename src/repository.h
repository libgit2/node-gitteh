#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"
#include "object_store.h"

namespace gitteh {

class Tree;
class Tag;
class Commit;
class Index;

class Repository : public ObjectWrap {
public:
	~Repository();
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Tree *wrapTree(git_tree*);
	Tag *wrapTag(git_tag*);
	Commit *wrapCommit(git_commit*);
	
	git_repository *repo_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetODB(const Arguments&);
	static Handle<Value> GetCommit(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> GetTag(const Arguments&);
	static Handle<Value> GetRawObject(const Arguments&);
	static Handle<Value> CreateWalker(const Arguments&);
	static Handle<Value> IndexGetter(Local<String>, const AccessorInfo&);

	static Handle<Value> Exists(const Arguments&);
	
	static Handle<Value> CreateTag(const Arguments&);
	static Handle<Value> CreateTree(const Arguments&);
	static Handle<Value> CreateCommit(const Arguments&);

	void close();

	ObjectStore<Commit, git_commit> commitStore_;
	ObjectStore<Tree, git_tree> treeStore_;
	ObjectStore<Tag, git_tag> tagStore_;

	Index *index_;
	char *path_;

	git_odb *odb_;
};

} // namespace gitteh

#endif // GITTEH_REPO_H
