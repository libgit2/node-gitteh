#ifndef GITTEH_INDEX_ENTRY_H
#define GITTEH_INDEX_ENTRY_H

#include "gitteh.h"
#include "ts_objectwrap.h"

namespace gitteh {

class Index;

class IndexEntry : public ThreadSafeObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	void setOwner(void *owner);

protected:
	static Handle<Value> New(const Arguments&);

	void processInitData(void *data);
	void *loadInitData();

	git_index_entry *entry_;

private:
	Index *index_;
};

} // namespace gitteh

#endif // GITTEH_INDEX_ENTRY_H
