#ifndef GITTEH_REF_H
#define GITTEH_REF_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

class Repository;
struct ref_data;

class Reference : public WrappedGitObject<Reference, git_reference> {
public:
	Reference(git_reference*);

	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	void setOwner(void*);

	void lock();
	void unlock();

	Repository *repository_;
	git_reference *ref_;
	bool deleted_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Rename(const Arguments&);
	static Handle<Value> Delete(const Arguments&);
	static Handle<Value> Resolve(const Arguments&);
	static Handle<Value> SetTarget(const Arguments&);

	void processInitData();
	int doInit();

	git_ref_t type_;

private:
	static void EIO_Rename(eio_req*);
	static int EIO_AfterRename(eio_req*);

	static void EIO_Delete(eio_req*);
	static int EIO_AfterDelete(eio_req*);

	static void EIO_Resolve(eio_req*);
	static int EIO_AfterResolve(eio_req*);

	static void EIO_SetTarget(eio_req*);
	static int EIO_AfterSetTarget(eio_req*);

	gitteh_lock lock_;
	ref_data *initData_;
};

} // namespace gitteh

#endif //GITTEH_REF_H
