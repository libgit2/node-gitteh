#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"
#include "gitobjectwrap.h"

namespace gitteh {

class Repository;

class Tag : public GitObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	void setOwner(void*);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData(void*);
	void* loadInitData();

	git_tag *tag_;

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);
};

} // namespace gitteh

#endif	// GITTEH_TAG_H
