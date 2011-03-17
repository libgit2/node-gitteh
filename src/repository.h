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
class RevWalker;

template <class, class, class> class ObjectFactory;

class Repository : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	template<class, class,class> friend class ObjectFactory;

	Repository();
	~Repository();
	static void Init(Handle<Object>);

	// Big ugly hacks, I hope to remove these someday. Pretty much any operation
	// on a libgit2 repository needs to be locked to one thread at a time, as
	// libgit2 is not thread safe in the slightest.
	void lockRepository();
	void unlockRepository();

	ObjectFactory<Repository, Commit, git_commit> *commitFactory_;
	ObjectFactory<Repository, Tag, git_tag> *tagFactory_;
	ObjectFactory<Repository, Tree, git_tree> *treeFactory_;
	ObjectFactory<Repository, RawObject, git_rawobj> *rawObjFactory_;
	ObjectFactory<Repository, Reference, git_reference> *referenceFactory_;

	git_repository *repo_;
	git_odb *odb_;

protected:
	static Handle<Value> OpenRepository(const Arguments&);
	static Handle<Value> InitRepository(const Arguments&);

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

	static Handle<Value> ListReferences(const Arguments&);

	void close();

	Index *index_;
	char *path_;

private:
	int getTree(git_oid*, git_tree**);
	int getTag(git_oid*, git_tag**);
	int getCommit(git_oid*, git_commit**);
	int getReference(const char*, git_reference**);
	int getRawObject(git_oid*, git_rawobj**);

	RevWalker *wrapRevWalker(git_revwalk*);

	int createTree(git_tree**);
	int createCommit(git_commit**);
	int createTag(git_tag**);
	int createRawObject(git_rawobj**);
	int createRevWalker(git_revwalk**);

	static int EIO_Exists(eio_req*);
	static int EIO_AfterExists(eio_req*);

	static int EIO_OpenRepository(eio_req*);
	static int EIO_AfterOpenRepository(eio_req*);

	static int EIO_InitRepository(eio_req*);
	static int EIO_AfterInitRepository(eio_req*);

	static int EIO_GetCommit(eio_req*);
	static int EIO_CreateCommit(eio_req*);
	static int EIO_ReturnCommit(eio_req*);
	
	static int EIO_GetTree(eio_req*);
	static int EIO_CreateTree(eio_req*);
	static int EIO_ReturnTree(eio_req*);
	
	static int EIO_GetTag(eio_req*);
	static int EIO_CreateTag(eio_req*);
	static int EIO_ReturnTag(eio_req*);
	
	static int EIO_GetRawObject(eio_req*);
	static int EIO_CreateRawObject(eio_req*);
	static int EIO_ReturnRawObject(eio_req*);

	static int EIO_GetReference(eio_req*);
	static int EIO_CreateSymbolicRef(eio_req*);
	static int EIO_CreateOidRef(eio_req*);
	static int EIO_ReturnReference(eio_req*);

	static int EIO_CreateRevWalker(eio_req*);
	static int EIO_ReturnRevWalker(eio_req*);

	static int EIO_GetRefList(eio_req*);
	static int EIO_AfterGetRefList(eio_req*);

	// For now, I'm using one lock for anything that calls a git_* api function.
	// I could probably have different locks for different sections of libgit2,
	// as I'm sure working on the index file or working on a specific ref isn't
	// going to step on the toes of a simultaneous call to get a tree entry for
	// example. However for now I want this thing to *just work*, I'll worry 
	// about making it a speed demon later. Ideally libgit2 will become thread
	// safe internally, then I can remove all this shit!
	gitteh_lock gitLock_;
};

} // namespace gitteh

#endif // GITTEH_REPO_H
