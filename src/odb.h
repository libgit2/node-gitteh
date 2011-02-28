#ifndef GITTEH_ODB_H
#define GITTEH_ODB_H

#include "gitteh.h"

class ObjectDatabase : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

protected:
	ObjectDatabase();

	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Read(const Arguments& args);

	git_odb *odb_;
};

#endif // GITTEH_ODB_H
