#ifndef GITTEH_REF_H
#define GITTEH_REF_H

#include "gitteh.h"
#include "gitobjectwrap.h"

namespace gitteh {

class Repository;

class Reference : public GitObjectWrap {
public:
	Reference();

	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	void setOwner(void*);

	void lock();
	void unlock();

	Repository *repository_;
	bool deleted_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Rename(const Arguments&);
	static Handle<Value> Delete(const Arguments&);
	static Handle<Value> Resolve(const Arguments&);
	static Handle<Value> SetTarget(const Arguments&);

	void processInitData(void *data);
	void* loadInitData();

	git_reference *ref_;
	git_rtype type_;

private:
	static int EIO_Rename(eio_req*);
	static int EIO_AfterRename(eio_req*);

	static int EIO_Delete(eio_req*);
	static int EIO_AfterDelete(eio_req*);

	static int EIO_Resolve(eio_req*);
	static int EIO_AfterResolve(eio_req*);

	static int EIO_SetTarget(eio_req*);
	static int EIO_AfterSetTarget(eio_req*);

	gitteh_lock lock_;
};

} // namespace gitteh

#endif //GITTEH_REF_H
