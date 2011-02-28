#ifndef GITTEH_REPO_H
#define GITTEH_REPO_H

#include "gitteh.h"

#define REPO_INTERNAL_FIELD_COMMIT_STORE 1

class Repository : public ObjectWrap {
public:
	~Repository();
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> GetODB(const Arguments& args);
	static Handle<Value> GetCommit(const Arguments& args);
	void close();

	git_repository *repo_;
};

#endif // GITTEH_REPO_H
