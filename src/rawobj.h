#ifndef GITTEH_RAWOBJ_H
#define GITTEH_RAWOBJ_H

#include "gitteh.h"
#include <node_buffer.h>

#define RAWOBJ_TYPE_SYMBOL String::NewSymbol("type")
#define RAWOBJ_DATA_SYMBOL String::NewSymbol("data")

class RawObject : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> TypeGetter(Local<String> property, const AccessorInfo& info);
	static Handle<Value> DataGetter(Local<String> property, const AccessorInfo& info);

	git_rawobj obj_;
};

#endif	// GITTEH_RAWOBJ_H

