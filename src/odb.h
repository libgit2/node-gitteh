#ifndef GITTEH_ODB_H_
#define GITTEH_ODB_H

#include "gitteh.h"
#include "object_cache.h"

namespace gitteh {

class ObjectDatabase : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	ObjectDatabase();
	~ObjectDatabase();

	static void Init(Handle<Object>);

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Open(const Arguments& args);

	static Handle<Value> Exists(const Arguments& args);
	static Handle<Value> Get(const Arguments& args);

private:
	static int EIO_Open(eio_req*);
	static int EIO_AfterOpen(eio_req*);

	static int EIO_Get(eio_req*);
	static int EIO_AfterGet(eio_req*);

	inline void lockOdb() {
		LOCK_MUTEX(odbLock_);
	}

	inline void unlockOdb() {
		UNLOCK_MUTEX(odbLock_);
	}

	WrappedGitObjectCache<ODBObject, git_odb_object> *objectCache_;
	gitteh_lock odbLock_;
	git_odb *odb_;
	bool created_;	// If true, this ODB was created independently of a repo, and should be free'd once wrapping obj is destroyed.
};

} // namespace gitteh

#endif // GITTEH_ODB_H
