#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"
#include <map>

#define REPO_INTERNAL_FIELD_COMMIT_STORE 1

class Repository : public ObjectWrap {
public:
	~Repository();
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Handle<Object> wrapCommit(git_commit*);
	void notifyCommitDeath(git_commit*);

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> GetODB(const Arguments&);
	static Handle<Value> GetCommit(const Arguments&);
	void close();

	git_repository *repo_;
	std::map<int, void*> commitObjects;
};

#endif // GITTEH_REPO_H
