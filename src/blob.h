#ifndef GITTEH_BLOB_H
#define GITTEH_BLOB_H

#include "gitteh.h"
#include "gitobjectwrap.h"

namespace gitteh {

class Repository;

class Blob : public GitObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	static void Init(Handle<Object>);

	Blob();
	~Blob();

	void setOwner(void*);

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void *loadInitData();
	void processInitData(void*);

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);

	Repository *repository_;
	git_blob *blob_;
};

} // namespace gitteh

#endif // GITTEH_BLOB_H
