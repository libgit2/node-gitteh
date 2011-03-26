#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "gitobjectwrap.h"

namespace gitteh {

class Repository;

class Commit : public GitObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	Commit();
	~Commit();

	static Handle<Value> SaveObject(Handle<Object>, Repository*, Handle<Value>, bool);

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
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);
};

} // namespace gitteh

#endif // GITTEH_COMMIT_H
