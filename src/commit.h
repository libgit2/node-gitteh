#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "ts_objectwrap.h"

namespace gitteh {

struct commit_data {
	char id[40];
	char *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
};

class Repository;

class Commit : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	Commit();
	~Commit();

	void* loadInitData();

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

	int parentCount_;

private:
	static int EIO_GetParent(eio_req*);
	static int EIO_AfterGetParent(eio_req*);
};

} // namespace gitteh

#endif // GITTEH_COMMIT_H
