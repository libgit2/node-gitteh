#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"

class Tag : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

protected:
	static Handle<Value> New(const Arguments&);

	git_tag *tag_;
};

#endif	// GITTEH_TAG_H
