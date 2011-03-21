#ifndef GITTEH_INDEX_H
#define GITTEH_INDEX_H

#include "gitteh.h"
#include "ts_objectwrap.h"
#include "object_factory.h"

namespace gitteh {

class IndexEntry;
class Repository;
template <class, class, class> class ObjectFactory;

class Index : public ThreadSafeObjectWrap {
public:
	template<class, class,class> friend class ObjectFactory;

	static Persistent<FunctionTemplate> constructor_template;

	Index();
	~Index();

	static void Init(Handle<Object>);

	void setOwner(void*);

	int initError_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> GetEntry(const Arguments&);
	static Handle<Value> FindEntry(const Arguments&);
	static Handle<Value> AddEntry(const Arguments&);
	static Handle<Value> InsertEntry(const Arguments&);
	static Handle<Value> RemoveEntry(const Arguments&);
	static Handle<Value> Write(const Arguments&);

	void processInitData(void*);
	void *loadInitData();

	git_index *index_;
	ObjectFactory<Index, IndexEntry, git_index_entry> *entryFactory_;
	unsigned int entryCount_;

private:
	void updateEntryCount();

	static int EIO_GetEntry(eio_req*);
	static int EIO_AfterGetEntry(eio_req*);

	static int EIO_Write(eio_req*);
	static int EIO_AfterWrite(eio_req*);

	static int EIO_AddEntry(eio_req*);
	static int EIO_AfterAddEntry(eio_req*);

	Repository *repository_;
};

} // namespace gitteh

#endif // GITTEH_INDEX_H
