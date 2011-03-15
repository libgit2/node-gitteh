#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"
#include "ts_objectwrap.h"

namespace gitteh {

class Repository;

class Tag : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	void setOwner(void*);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData(void *data);
	void* loadInitData();

	git_tag *tag_;
};

} // namespace gitteh

#endif	// GITTEH_TAG_H
