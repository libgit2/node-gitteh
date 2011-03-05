#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"

namespace gitteh {

class Repository;

class Tag : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Save(const Arguments&);

	git_tag *tag_;
};

} // namespace gitteh

#endif	// GITTEH_TAG_H
