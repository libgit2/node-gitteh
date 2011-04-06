#ifndef GITTEH_ODB_H
#define GITTEH_ODB_H

#include "gitteh.h"
#include "object_cache.h"
#include "odb_obj.h"

namespace gitteh {

class ObjectDatabase : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	ObjectDatabase();
	~ObjectDatabase();

	inline void lockOdb() {
		LOCK_MUTEX(odbLock_);
	}

	inline void unlockOdb() {
		UNLOCK_MUTEX(odbLock_);
	}

	static void Init(Handle<Object>);

	git_odb *odb_;

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Open(const Arguments& args);

	static Handle<Value> Exists(const Arguments& args);
	static Handle<Value> Get(const Arguments& args);
	static Handle<Value> Create(const Arguments& args);

private:
	static int EIO_Open(eio_req*);
	static int EIO_AfterOpen(eio_req*);

	static int EIO_Get(eio_req*);
	static int EIO_AfterGet(eio_req*);

	WrappedGitObjectCache<ObjectDatabase, ODBObject, git_odb_object> *objectCache_;
	gitteh_lock odbLock_;
	bool created_;	// If true, this ODB was created independently of a repo, and should be free'd once wrapping obj is destroyed.
};

} // namespace gitteh

#endif // GITTEH_ODB_H
