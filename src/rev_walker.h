#ifndef GITTEH_REV_WALKER_H
#define GITTEH_REV_WALKER_H

#include "gitteh.h"

namespace gitteh {

class Repository;

class RevWalker : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	~RevWalker();

	Repository *repo_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Push(const Arguments&);
	static Handle<Value> Hide(const Arguments&);
	static Handle<Value> Next(const Arguments&);
	static Handle<Value> Sort(const Arguments&);
	static Handle<Value> Reset(const Arguments&);

	git_revwalk *walker_;
private:
	static int EIO_Push(eio_req*);
	static int EIO_AfterPush(eio_req*);

	static int EIO_Hide(eio_req*);
	static int EIO_AfterHide(eio_req*);

	static int EIO_Next(eio_req*);
	static int EIO_AfterNext(eio_req*);

	static int EIO_Sort(eio_req*);
	static int EIO_AfterSort(eio_req*);

	static int EIO_Reset(eio_req*);
	static int EIO_AfterReset(eio_req*);
};

} // namespace gitteh

#endif // GITTEH_REV_WALKER_H

