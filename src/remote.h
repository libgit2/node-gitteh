#ifndef GITTEH_REMOTE_H
#define GITTEH_REMOTE_H

#include "nan.h"
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
		static NAN_METHOD(New);
		static NAN_METHOD(UpdateTips);
		static NAN_METHOD(Connect);
		static NAN_METHOD(Download);

	private:
		git_remote *remote_;
		git_transfer_progress progress_;
		git_off_t downloadBytes_;

		static NAN_GETTER(GetStats);
		static void AsyncUpdateTips(uv_work_t*);
		static void AsyncAfterUpdateTips(uv_work_t*);
		static void AsyncConnect(uv_work_t*);
		static void AsyncAfterConnect(uv_work_t*);
		static int DownloadTransferProgressCallback(const git_transfer_progress*, void*);
		static void AsyncDownload(uv_work_t*);
		static void AsyncAfterDownload(uv_work_t*);
	};
};

#endif
