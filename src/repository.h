#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"
#include "object_store.h"

namespace gitteh {

class Tree;
class Tag;
class Commit;
class Index;
class RawObject;
class Reference;

class Repository : public ObjectWrap {
public:
	~Repository();
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Tree *wrapTree(git_tree*);
	Tag *wrapTag(git_tag*);
	Commit *wrapCommit(git_commit*);
	Reference *wrapReference(git_reference*);
	
	git_repository *repo_;
	git_odb *odb_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetODB(const Arguments&);
	static Handle<Value> GetCommit(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> GetTag(const Arguments&);
	static Handle<Value> GetRawObject(const Arguments&);
	static Handle<Value> GetReference(const Arguments&);


	static Handle<Value> IndexGetter(Local<String>, const AccessorInfo&);

	static Handle<Value> Exists(const Arguments&);

	static Handle<Value> CreateRawObject(const Arguments&);
	static Handle<Value> CreateTag(const Arguments&);
	static Handle<Value> CreateTree(const Arguments&);
	static Handle<Value> CreateCommit(const Arguments&);
	static Handle<Value> CreateWalker(const Arguments&);
	static Handle<Value> CreateSymbolicRef(const Arguments&);
	static Handle<Value> CreateOidRef(const Arguments&);

	void close();

	ObjectStore<Commit, git_commit> commitStore_;
	ObjectStore<Tree, git_tree> treeStore_;
	ObjectStore<Tag, git_tag> tagStore_;
	ObjectStore<Reference, git_reference> refStore_;

	Index *index_;
	char *path_;
};

} // namespace gitteh

#endif // GITTEH_REPO_H
