#ifndef GITTEH_INDEX_H
#define GITTEH_INDEX_H

#include "gitteh.h"
#include "ts_objectwrap.h"
#include "object_store.h"

namespace gitteh {

class IndexEntry;
class Repository;

class Index : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	~Index();

	static void Init(Handle<Object>);
	IndexEntry *wrapIndexEntry(git_index_entry*);

	void setOwner(void*);

	int initError_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> EntriesGetter(uint32_t, const AccessorInfo&);

	void processInitData(void*);
	void *loadInitData();

	git_index *index_;
	ObjectStore<IndexEntry, git_index_entry> entryStore_;
	unsigned int entryCount_;

private:
	Repository *repository_;
};

} // namespace gitteh

#endif // GITTEH_INDEX_H
