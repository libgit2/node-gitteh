#include "remote.h"
#include "git2/strarray.h"

using std::map;
using std::pair;

namespace gitteh {
	static Persistent<String> class_symbol;
	static Persistent<String> name_symbol;
	static Persistent<String> url_symbol;
	static Persistent<String> fetchspecs_symbol;
	static Persistent<String> pushspecs_symbol;
	static Persistent<String> progress_symbol;

	static Persistent<String> refspec_src_symbol;
	static Persistent<String> refspec_dst_symbol;

	static Persistent<String> progress_total_objects_symbol;
	static Persistent<String> progress_indexed_objects_symbol;
	static Persistent<String> progress_received_objects_symbol;
	static Persistent<String> progress_received_bytes_symbol;

	class RemoteBaton : public Baton {
	public:
		Remote *remote_;

		RemoteBaton(Remote *remote) : Baton(), remote_(remote) {
			remote_->Ref();
		}

		~RemoteBaton() {
			remote_->Unref();
		}
	};

	class UpdateTipsBaton : public RemoteBaton {
	public:
		UpdateTipsBaton(Remote *remote) : RemoteBaton(remote) { }
	};

	class ConnectBaton : public RemoteBaton {
	public:
		git_direction direction;
		map<string, git_oid> refs;

		ConnectBaton(Remote *remote, int direction) :
				RemoteBaton(remote), direction((git_direction)direction) { }
	};

	class DownloadBaton : public RemoteBaton {
	public:
		git_off_t *bytes;
		git_transfer_progress *stats;
		DownloadBaton(Remote *remote) :
				RemoteBaton(remote) { }
	};

	static int SaveRemoteRef(git_remote_head *head, void *payload) {
		ConnectBaton *baton = static_cast<ConnectBaton*>(payload);
		baton->refs.insert(pair<string, git_oid>(string(head->name), head->oid));
		return GIT_OK;
	}

	Persistent<FunctionTemplate> Remote::constructor_template;

	Remote::Remote(git_remote *remote) : ObjectWrap() {
		remote_ = remote;
	}

	Remote::~Remote() {
		if(remote_ != NULL) {
			git_remote_free(remote_);
			remote_ = NULL;
		}
	}

	void Remote::Init(Handle<Object> target) {
		NanScope();

		class_symbol 		= NODE_PSYMBOL("NativeRemote");
		name_symbol 		= NODE_PSYMBOL("name");
		url_symbol 			= NODE_PSYMBOL("url");
		fetchspecs_symbol	= NODE_PSYMBOL("fetchSpecs");
		pushspecs_symbol 	= NODE_PSYMBOL("pushSpecs");
		progress_symbol 	= NODE_PSYMBOL("progress");

		refspec_src_symbol 	= NODE_PSYMBOL("src");
		refspec_dst_symbol 	= NODE_PSYMBOL("dst");

		progress_total_objects_symbol    = NODE_PSYMBOL("totalObjects");
		progress_indexed_objects_symbol  = NODE_PSYMBOL("indexedObjects");
		progress_received_objects_symbol = NODE_PSYMBOL("receivedObjects");
		progress_received_bytes_symbol   = NODE_PSYMBOL("receivedBytes");

		Local<FunctionTemplate> t = NanNew<FunctionTemplate>(New);
		NanAssignPersistent(constructor_template, t);
		constructor_template->SetClassName(class_symbol);
		t->InstanceTemplate()->SetInternalFieldCount(1);

		NODE_SET_PROTOTYPE_METHOD(t, "updateTips", UpdateTips);
		NODE_SET_PROTOTYPE_METHOD(t, "connect", Connect);
		NODE_SET_PROTOTYPE_METHOD(t, "download", Download);

		target->Set(class_symbol, constructor_template->GetFunction());
	}

	NAN_METHOD(Remote::New) {
		NanEscapableScope();
		REQ_EXT_ARG(0, remoteArg);
		Handle<Object> me = args.This();

		git_remote *remote = static_cast<git_remote*>(remoteArg->Value());
		Remote *remoteObj = new Remote(remote);
		remoteObj->Wrap(me);

		// Get the fetch- and push-specs
		Handle<Array> fetchspecsArr = NanNew<Array>();
		git_strarray fetchspecs = {NULL, 0};
		if (!git_remote_get_fetch_refspecs(&fetchspecs, remote)) {
			for (size_t i=0; i<fetchspecs.count; i++)
				fetchspecsArr->Set(i, CastToJS(fetchspecs.strings[i]));
		}
		me->Set(fetchspecs_symbol, fetchspecsArr);

		Handle<Array> pushspecsArr = NanNew<Array>();
		git_strarray pushspecs = {NULL, 0};
		if (!git_remote_get_push_refspecs(&pushspecs, remote)) {
			for (size_t i=0; i<pushspecs.count; i++)
				pushspecsArr->Set(i, CastToJS(pushspecs.strings[i]));
		}
		me->Set(pushspecs_symbol, pushspecsArr);

		me->Set(name_symbol, CastToJS(git_remote_name(remote)));
		me->Set(url_symbol, CastToJS(git_remote_url(remote)));

		return NanEscapeScope(me);
	}

	NAN_METHOD(Remote::UpdateTips) {
		NanScope();
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		UpdateTipsBaton *baton = new UpdateTipsBaton(remote);
		baton->setCallback(args[0]);
		uv_queue_work(uv_default_loop(), &baton->req, AsyncUpdateTips,
				NODE_094_UV_AFTER_WORK_CAST(AsyncAfterUpdateTips));
		return NanUndefined();
	}

	void Remote::AsyncUpdateTips(uv_work_t *req) {
		UpdateTipsBaton *baton = GetBaton<UpdateTipsBaton>(req);
		// TODO: use the callback to get changed refs once fn accepts a payload
		AsyncLibCall(git_remote_update_tips(baton->remote_->remote_),
				baton);
	}

	void Remote::AsyncAfterUpdateTips(uv_work_t *req) {
		NanScope();
		UpdateTipsBaton *baton = GetBaton<UpdateTipsBaton>(req);

		if(baton->isErrored()) {
			Handle<Value> argv[] = { baton->createV8Error() };
			FireCallback(baton->callback, 1, argv);
		}
		else {
			Handle<Value> argv[] = { Undefined() };
			FireCallback(baton->callback, 1, argv);
		}

		delete baton;
	}

	NAN_METHOD(Remote::Connect) {
		NanScope();
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		ConnectBaton *baton = new ConnectBaton(remote, CastFromJS<int>(args[0]));
		baton->setCallback(args[1]);
		uv_queue_work(uv_default_loop(), &baton->req, AsyncConnect,
				NODE_094_UV_AFTER_WORK_CAST(AsyncAfterConnect));
		return NanUndefined();
	}

	void Remote::AsyncConnect(uv_work_t *req) {
		ConnectBaton *baton = GetBaton<ConnectBaton>(req);
		if(AsyncLibCall(git_remote_connect(baton->remote_->remote_,
				baton->direction), baton)) {
			git_remote_ls(baton->remote_->remote_, SaveRemoteRef, baton);
		}
	}

	void Remote::AsyncAfterConnect(uv_work_t *req) {
		HandleScope scope;
		ConnectBaton *baton = GetBaton<ConnectBaton>(req);

		if(baton->isErrored()) {
			Handle<Value> argv[] = { baton->createV8Error() };
			FireCallback(baton->callback, 1, argv);
		}
		else {
			Handle<Value> argv[] = { Undefined(), CastToJS(baton->refs) };
			FireCallback(baton->callback, 2, argv);
		}

		delete baton;
	}

	NAN_METHOD(Remote::Download) {
		NanScope();
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		DownloadBaton *baton = new DownloadBaton(remote);
		baton->setCallback(args[0]);
		baton->bytes = &remote->downloadBytes_;
		baton->stats = &remote->progress_;

		// Re-initialize stat counters.
		remote->downloadBytes_ = 0;
		memset(&remote->progress_, 0, sizeof(git_transfer_progress));

		// Setup download stats accessor.
		remote->handle_->SetAccessor(progress_symbol, GetStats);

		uv_queue_work(uv_default_loop(), &baton->req, AsyncDownload,
				NODE_094_UV_AFTER_WORK_CAST(AsyncAfterDownload));
		return NanUndefined();
	}

	int Remote::DownloadTransferProgressCallback(
			const git_transfer_progress *stats,
			void *payload)
	{
		DownloadBaton *baton = (DownloadBaton*)payload;
		baton->remote_->progress_ = *stats;
		return 0;
	}

	void Remote::AsyncDownload(uv_work_t *req) {
		DownloadBaton *baton = GetBaton<DownloadBaton>(req);
		AsyncLibCall(git_remote_download(baton->remote_->remote_,
				DownloadTransferProgressCallback, baton), baton);
	}

	void Remote::AsyncAfterDownload(uv_work_t *req) {
		HandleScope scope;
		DownloadBaton *baton = GetBaton<DownloadBaton>(req);

		baton->remote_->handle_->Delete(progress_symbol);

		if(baton->isErrored()) {
			Handle<Value> argv[] = { baton->createV8Error() };
			FireCallback(baton->callback, 1, argv);
		}
		else {
			Handle<Value> argv[] = { Undefined() };
			FireCallback(baton->callback, 1, argv);
		}

		delete baton;
	}

	NAN_GETTER(Remote::GetStats) {
		NanEscapableScope();
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		Handle<Object> o = NanNew<Object>();
		o->Set(progress_total_objects_symbol, CastToJS(remote->progress_.total_objects));
		o->Set(progress_indexed_objects_symbol, CastToJS(remote->progress_.indexed_objects));
		o->Set(progress_received_objects_symbol, CastToJS(remote->progress_.received_objects));
		o->Set(progress_received_bytes_symbol, CastToJS(remote->progress_.received_bytes));
		return NanEscapeScope(o);
	}

};	// namespace gitteh


namespace cvv8 {
	template<>
	struct NativeToJS<git_refspec> {
		Handle<Value> operator() (git_refspec const *refspec) const {
			NanEscapableScope();
			Handle<Object> o = NanNew<Object>();
			o->Set(gitteh::refspec_src_symbol, CastToJS(git_refspec_src(refspec)));
			o->Set(gitteh::refspec_dst_symbol, CastToJS(git_refspec_dst(refspec)));
			return NanEscapeScope(o);
		}
	};
}
