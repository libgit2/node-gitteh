#include "remote.h"

static Persistent<String> class_symbol;
static Persistent<String> name_symbol;
static Persistent<String> url_symbol;
static Persistent<String> fetchspec_symbol;
static Persistent<String> pushspec_symbol;

static Persistent<String> refspec_src_symbol;
static Persistent<String> refspec_dst_symbol;

namespace cvv8 {
	template<> 
	struct NativeToJS<git_refspec> {
		Handle<Value> operator() (git_refspec const *refspec) const {
			HandleScope scope;
			Handle<Object> o = Object::New();
			o->Set(refspec_src_symbol, CastToJS(git_refspec_src(refspec)));
			o->Set(refspec_dst_symbol, CastToJS(git_refspec_dst(refspec)));
			return scope.Close(o);
		}
	};
}

namespace gitteh {
	Persistent<FunctionTemplate> Remote::constructor_template;
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

	Remote::Remote(git_remote *remote) : ObjectWrap() {
		remote_ = remote;
	}

	Remote::~Remote() {
		if(remote_ != NULL) {
			git_remote_free(remote_);
			remote_ = NULL;
		}
	}
};	// namespace gitteh