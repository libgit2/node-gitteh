#ifndef GITTEH_BLOB_H
#define GITTEH_BLOB_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

class Repository;

struct blob_data;

class Blob : public WrappedGitObject<Blob, git_blob> {
public:
	static Persistent<FunctionTemplate> constructor_template;

	static void Init(Handle<Object>);
	static Handle<Value> SaveObject(Handle<Object>, Repository*, Handle<Value>, bool);

	Blob(git_blob*);
	~Blob();

	void setOwner(void*);

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	int doInit();
	void processInitData();

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);

	Repository *repository_;
	git_blob *blob_;
	blob_data *initData_;
};

} // namespace gitteh

#endif // GITTEH_BLOB_H
