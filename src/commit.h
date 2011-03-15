#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "ts_objectwrap.h"

namespace gitteh {

class Repository;

class Commit : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	Commit();
	~Commit();

	void setOwner(void*);

	Repository *repository_;
	git_commit *commit_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> SetTree(const Arguments&);
	static Handle<Value> AddParent(const Arguments&);
	static Handle<Value> GetParent(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData(void *data);
	void* loadInitData();

	int parentCount_;

private:
	static int EIO_AddParent(eio_req*);
	static int EIO_AfterAddParent(eio_req*);

	static int EIO_GetParent(eio_req*);
	static int EIO_AfterGetParent(eio_req*);

	static int EIO_GetTree(eio_req*);
	static int EIO_AfterGetTree(eio_req*);

	static int EIO_SetTree(eio_req*);
	static int EIO_AfterSetTree(eio_req*);

	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);
};

} // namespace gitteh

#endif // GITTEH_COMMIT_H
