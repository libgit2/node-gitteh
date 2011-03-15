#ifndef GITTEH_RAWOBJ_H
#define GITTEH_RAWOBJ_H

#include "gitteh.h"
#include "ts_objectwrap.h"

namespace gitteh {

#define RAWOBJ_TYPE_SYMBOL String::NewSymbol("type")
#define RAWOBJ_DATA_SYMBOL String::NewSymbol("data")

class Repository;

class RawObject : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

	~RawObject();

	void setOwner(void*);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Save(const Arguments& args);

	void processInitData(void *data);
	void* loadInitData();

	git_rawobj *obj_;
};

} // namespace gitteh

#endif	// GITTEH_RAWOBJ_H
