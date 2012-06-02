#include "remote.h"

namespace gitteh {
	static Persistent<String> class_symbol;
	static Persistent<String> name_symbol;
	static Persistent<String> url_symbol;
	static Persistent<String> fetchspec_symbol;
	static Persistent<String> pushspec_symbol;
	static Persistent<String> stats_symbol;

	static Persistent<String> refspec_src_symbol;
	static Persistent<String> refspec_dst_symbol;

	static Persistent<String> stats_bytes_symbol;
	static Persistent<String> stats_total_symbol;
	static Persistent<String> stats_done_symbol;

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
		int direction;
		std::list<string> refs;

		ConnectBaton(Remote *remote, int direction) :
				RemoteBaton(remote), direction(direction) { }
	};

	class DownloadBaton : public RemoteBaton {
	public:
		git_off_t *bytes;
		git_indexer_stats *stats;
		DownloadBaton(Remote *remote) :
				RemoteBaton(remote) { }
	};

	static int SaveRemoteRef(git_remote_head *head, void *payload) {
		ConnectBaton *baton = static_cast<ConnectBaton*>(payload);
		baton->refs.push_back(string(head->name));
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
		HandleScope scope;

		class_symbol 		= NODE_PSYMBOL("NativeRemote");
		name_symbol 		= NODE_PSYMBOL("name");
		url_symbol 			= NODE_PSYMBOL("url");
		fetchspec_symbol 	= NODE_PSYMBOL("fetchSpec");
		pushspec_symbol 	= NODE_PSYMBOL("pushSpec");
		stats_symbol 		= NODE_PSYMBOL("stats");

		refspec_src_symbol 	= NODE_PSYMBOL("src");
		refspec_dst_symbol 	= NODE_PSYMBOL("dst");

		stats_bytes_symbol	= NODE_PSYMBOL("bytes");
		stats_total_symbol	= NODE_PSYMBOL("total");
		stats_done_symbol	= NODE_PSYMBOL("done");

		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(class_symbol);
		t->InstanceTemplate()->SetInternalFieldCount(1);

		NODE_SET_PROTOTYPE_METHOD(t, "updateTips", UpdateTips);
		NODE_SET_PROTOTYPE_METHOD(t, "connect", Connect);
		NODE_SET_PROTOTYPE_METHOD(t, "download", Download);

		target->Set(class_symbol, constructor_template->GetFunction());
	}

	Handle<Value> Remote::New(const Arguments& args) {
		HandleScope scope;
		REQ_EXT_ARG(0, remoteArg);
		Handle<Object> me = args.This();

		git_remote *remote = static_cast<git_remote*>(remoteArg->Value());
		Remote *remoteObj = new Remote(remote);
		remoteObj->Wrap(me);

		me->Set(name_symbol, CastToJS(git_remote_name(remote)));
		me->Set(url_symbol, CastToJS(git_remote_url(remote)));
		me->Set(fetchspec_symbol, CastToJS(git_remote_fetchspec(remote)));
		me->Set(pushspec_symbol, CastToJS(git_remote_pushspec(remote)));

		return scope.Close(me);
	}

	Handle<Value> Remote::UpdateTips(const Arguments& args) {
		HandleScope scope;
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		UpdateTipsBaton *baton = new UpdateTipsBaton(remote);
		baton->setCallback(args[0]);
		uv_queue_work(uv_default_loop(), &baton->req, AsyncUpdateTips, 
				AsyncAfterUpdateTips);
		return Undefined();
	}

	void Remote::AsyncUpdateTips(uv_work_t *req) {
		UpdateTipsBaton *baton = GetBaton<UpdateTipsBaton>(req);
		// TODO: use the callback to get changed refs once fn accepts a payload
		AsyncLibCall(git_remote_update_tips(baton->remote_->remote_, NULL),
				baton);
	}

	void Remote::AsyncAfterUpdateTips(uv_work_t *req) {
		HandleScope scope;
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

	Handle<Value> Remote::Connect(const Arguments& args) {
		HandleScope scope;
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		ConnectBaton *baton = new ConnectBaton(remote, CastFromJS<int>(args[0]));
		baton->setCallback(args[1]);
		uv_queue_work(uv_default_loop(), &baton->req, AsyncConnect, 
				AsyncAfterConnect);
		return Undefined();
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

	Handle<Value> Remote::Download(const Arguments &args) {
		HandleScope scope;
		Remote *remote = ObjectWrap::Unwrap<Remote>(args.This());
		DownloadBaton *baton = new DownloadBaton(remote);
		baton->setCallback(args[0]);
		baton->bytes = &remote->downloadBytes_;
		baton->stats = &remote->indexerStats_;

		// Re-initialize stat counters.
		remote->downloadBytes_ = 0;
		memset(&remote->indexerStats_, 0, sizeof(git_indexer_stats));

		// Setup download stats accessor.
		remote->handle_->SetAccessor(stats_symbol, GetStats);

		uv_queue_work(uv_default_loop(), &baton->req, AsyncDownload, 
				AsyncAfterDownload);
		return Undefined();
	}

	void Remote::AsyncDownload(uv_work_t *req) {
		DownloadBaton *baton = GetBaton<DownloadBaton>(req);
		AsyncLibCall(git_remote_download(baton->remote_->remote_,
				baton->bytes, baton->stats), baton);
	}

	void Remote::AsyncAfterDownload(uv_work_t *req) {
		HandleScope scope;
		DownloadBaton *baton = GetBaton<DownloadBaton>(req);

		baton->remote_->handle_->Delete(stats_symbol);

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

	Handle<Value> Remote::GetStats(Local<String> property, const AccessorInfo &info) {
		HandleScope scope;
		Remote *remote = ObjectWrap::Unwrap<Remote>(info.This());
		Handle<Object> o = Object::New();
		o->Set(stats_bytes_symbol, CastToJS(remote->downloadBytes_));
		o->Set(stats_total_symbol, CastToJS(remote->indexerStats_.total));
		o->Set(stats_done_symbol, CastToJS(remote->indexerStats_.processed));
		return scope.Close(o);
	}

};	// namespace gitteh


namespace cvv8 {
	template<> 
	struct NativeToJS<git_refspec> {
		Handle<Value> operator() (git_refspec const *refspec) const {
			HandleScope scope;
			Handle<Object> o = Object::New();
			o->Set(gitteh::refspec_src_symbol, CastToJS(git_refspec_src(refspec)));
			o->Set(gitteh::refspec_dst_symbol, CastToJS(git_refspec_dst(refspec)));
			return scope.Close(o);
		}
	};
}