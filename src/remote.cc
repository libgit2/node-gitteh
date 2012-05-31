#include "remote.h"

namespace gitteh {
	static Persistent<String> class_symbol;
	static Persistent<String> name_symbol;
	static Persistent<String> url_symbol;
	static Persistent<String> fetchspec_symbol;
	static Persistent<String> pushspec_symbol;

	static Persistent<String> refspec_src_symbol;
	static Persistent<String> refspec_dst_symbol;

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

		refspec_src_symbol 	= NODE_PSYMBOL("src");
		refspec_dst_symbol 	= NODE_PSYMBOL("dst");

		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(class_symbol);
		t->InstanceTemplate()->SetInternalFieldCount(1);

		NODE_SET_PROTOTYPE_METHOD(t, "updateTips", UpdateTips);

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