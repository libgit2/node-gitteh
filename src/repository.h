#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"

namespace gitteh {

class RepositoryBaton;

class Repository : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	friend class RepositoryBaton;
	// template<class, class,class> friend class ObjectFactory;

	Repository();
	~Repository();
	static void Init(Handle<Object>);

	// Big ugly hacks, I hope to remove these someday. Pretty much any operation
	// on a libgit2 repository needs to be locked to one thread at a time, as
	// libgit2 is not thread safe in the slightest.
	void lockRepository();
	void unlockRepository();

	git_repository *repo_;
	git_odb *odb_;
	git_index *index_;

protected:
	static Handle<Value> OpenRepository(const Arguments&);
	static Handle<Value> InitRepository(const Arguments&);

	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetObject(const Arguments&);
	static Handle<Value> GetReference(const Arguments&);
	static Handle<Value> CreateOidReference(const Arguments&);
	static Handle<Value> CreateSymReference(const Arguments&);
	static Handle<Value> GetRemote(const Arguments&);
	static Handle<Value> Exists(const Arguments&);
	static Handle<Value> CreateRemote(const Arguments&);

	void close();

private:
	Handle<Value> wrapObject(git_object*);

	static void AsyncOpenRepository(uv_work_t*);
	static void AsyncAfterOpenRepository(uv_work_t*);
	static void AsyncExists(uv_work_t*);
	static void AsyncAfterExists(uv_work_t*);
	static void AsyncInitRepository(uv_work_t*);
	static void AsyncAfterInitRepository(uv_work_t*);
	static void AsyncGetObject(uv_work_t*);
	static void AsyncAfterGetObject(uv_work_t*);
	static void AsyncGetReference(uv_work_t*);
	static void AsyncCreateReference(uv_work_t*);
	static void AsyncReturnReference(uv_work_t*);
	static void AsyncGetRemote(uv_work_t*);
	static void AsyncAfterGetRemote(uv_work_t*);
	static void AsyncCreateRemote(uv_work_t*);
	static void AsyncAfterCreateRemote(uv_work_t*);

	static Handle<Object> CreateReferenceObject(git_reference*);
	
	// For now, I'm using one lock for anything that calls a git_* api function.
	// I could probably have different locks for different sections of libgit2,
	// as I'm sure working on the index file or working on a specific ref isn't
	// going to step on the toes of a simultaneous call to get a tree entry for
	// example. However for now I want this thing to *just work*, I'll worry 
	// about making it a speed demon later. Ideally libgit2 will become thread
	// safe internally, then I can remove all this shit!
	gitteh_lock gitLock_;

	// This lock is used during ref packing. The problem is ref packing will
	// invalidate our previously cached pointers. Ugh. So what we do is update
	// those pointers after we pack references right? Cool. Only thing is with
	// async that process might get fucked if we don't stop any more refs from
	// being opened created while we're in the process of packing. Hence this lock.
	// gitteh_lock refLock_;
};

} // namespace gitteh

#endif // GITTEH_REPO_H
