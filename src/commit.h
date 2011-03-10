#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"

namespace gitteh {

struct commit_data {
	git_commit *commit;
	char id[40];
	char *message;
	git_signature *author;
	git_signature *committer;
	int parentCount;
};

class Repository;

class Commit : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	~Commit();

	void load(commit_data*);

	Repository *repository_;
	git_commit *commit_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> SetTree(const Arguments&);
	static Handle<Value> AddParent(const Arguments&);
	static Handle<Value> GetParent(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	int parentCount_;

private:
	static int EIO_GetParent(eio_req*);
	static int EIO_AfterGetParent(eio_req*);
};

} // namespace gitteh

#endif // GITTEH_COMMIT_H
