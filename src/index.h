#ifndef GITTEH_INDEX_H
#define GITTEH_INDEX_H

#include "nan.h"
#include "gitteh.h"

namespace gitteh {
	class IndexBaton;
	class Repository;

	class Index : public ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor_template;

		friend class IndexBaton;

		Index(Repository*, git_index*);
		~Index();
		static void Init(Handle<Object>);
	protected:
		static NAN_METHOD(New);
		static NAN_METHOD(ReadTree);
		static NAN_METHOD(Write);
	private:
		Repository *repository_;
		git_index *index_;

		static void AsyncReadTree(uv_work_t*);
		static void AsyncAfterReadTree(uv_work_t*);
		static void AsyncWrite(uv_work_t*);
		static void AsyncAfterWrite(uv_work_t*);
	};

} // namespace gitteh

#endif // GITTEH_INDEX_H
