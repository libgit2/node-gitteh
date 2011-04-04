#ifndef GITTEH_ODB_H_
#define GITTEH_ODB_H

#include "gitteh.h"

namespace gitteh {

class ObjectDatabase : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	ObjectDatabase();
	~ObjectDatabase();

	static void Init(Handle<Object>);

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> Open(const Arguments& args);

	static Handle<Value> Exists(const Arguments& args);

private:
	static int EIO_Open(eio_req*);
	static int EIO_AfterOpen(eio_req*);

	git_odb *odb_;
	bool created_;	// If true, this ODB was created independently of a repo, and should be free'd once wrapping obj is destroyed.
};

} // namespace gitteh

#endif // GITTEH_ODB_H
