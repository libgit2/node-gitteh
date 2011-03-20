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

	void processInitData(void*);
	void *loadInitData();

	git_index *index_;
	ObjectFactory<Index, IndexEntry, git_index_entry> *entryFactory_;
	unsigned int entryCount_;

private:
	static int EIO_GetEntry(eio_req*);
	static int EIO_AfterGetEntry(eio_req*);

	Repository *repository_;
};

} // namespace gitteh

#endif // GITTEH_INDEX_H
