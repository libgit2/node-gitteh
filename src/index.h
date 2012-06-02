#ifndef GITTEH_INDEX_H
#define GITTEH_INDEX_H

#include "gitteh.h"

namespace gitteh {
	class IndexBaton;
	class Index : public ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor_template;

		friend class IndexBaton;

		Index(git_index*);
		~Index();
		static void Init(Handle<Object>);
	protected:
		static Handle<Value> New(const Arguments&);
	private:
		git_index *index_;
	};

} // namespace gitteh

#endif // GITTEH_INDEX_H
