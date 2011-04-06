#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

struct commit_data;
class Repository;

class Commit : public WrappedGitObject<Commit, git_commit> {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	Commit(git_commit*);
	~Commit();

	static Handle<Value> SaveObject(Handle<Object>, Repository*, Handle<Value>, bool);

	void setOwner(Repository*);

	Repository *repository_;
	git_commit *commit_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetTree(const Arguments&);
	static Handle<Value> SetTree(const Arguments&);
	static Handle<Value> AddParent(const Arguments&);
	static Handle<Value> GetParent(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData();
	int doInit();

	int parentCount_;

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);

	void updateCachedRef(const git_oid*);

	commit_data *initData_;
};

} // namespace gitteh

#endif // GITTEH_COMMIT_H
