#ifndef GITTEH_REMOTE_H
#define GITTEH_REMOTE_H

#include "gitteh.h"

namespace gitteh {
	class RemoteBaton;

	class Remote : public ObjectWrap {
	public:
		friend class RemoteBaton;

		static Persistent<FunctionTemplate> constructor_template;
		static void Init(Handle<Object>);
		Remote(git_remote*);
		~Remote();

	protected:
		static Handle<Value> New(const Arguments&);
		static Handle<Value> UpdateTips(const Arguments&);
		static Handle<Value> Connect(const Arguments&);
		static Handle<Value> Download(const Arguments&);

	private:
		git_remote *remote_;
		git_indexer_stats indexerStats_;
		git_off_t downloadBytes_;

		static Handle<Value> GetStats(Local<String>, const AccessorInfo&);
		static void AsyncUpdateTips(uv_work_t*);
		static void AsyncAfterUpdateTips(uv_work_t*);
		static void AsyncConnect(uv_work_t*);
		static void AsyncAfterConnect(uv_work_t*);
		static void AsyncDownload(uv_work_t*);
		static void AsyncAfterDownload(uv_work_t*);
	};
};

#endif
