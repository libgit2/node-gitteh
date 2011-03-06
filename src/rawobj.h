#ifndef GITTEH_RAWOBJ_H
#define GITTEH_RAWOBJ_H

#include "gitteh.h"

namespace gitteh {

#define RAWOBJ_TYPE_SYMBOL String::NewSymbol("type")
#define RAWOBJ_DATA_SYMBOL String::NewSymbol("data")

class Repository;

class RawObject : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments& args);

	static Handle<Value> Save(const Arguments& args);

	git_rawobj *obj_;
};

} // namespace gitteh

#endif	// GITTEH_RAWOBJ_H
