#ifndef GITTEH_INDEX_ENTRY_H
#define GITTEH_INDEX_ENTRY_H

#include "gitteh.h"

namespace gitteh {

class IndexEntry : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

protected:
	static Handle<Value> New(const Arguments&);

	git_index_entry *entry_;
};

} // namespace gitteh

#endif // GITTEH_INDEX_ENTRY_H
