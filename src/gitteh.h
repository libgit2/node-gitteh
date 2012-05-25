#ifndef GITTEH_H
#define GITTEH_H

#include <v8.h>
#include "cvv8/convert.hpp"
#include <node.h>
#include <git2.h>
#include <node_object_wrap.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "thread.h"
#include "baton.h"

using namespace v8;
using namespace node;
using namespace cvv8;
using std::string;

#define REQ_EXT_ARG(I, VAR)                                             	\
	if (args.Length() <= (I) || !args[I]->IsExternal())                   	\
		return ThrowException(Exception::TypeError(                         \
			String::New("Argument " #I " invalid")));						\
	Local<External> VAR = Local<External>::Cast(args[I]);

namespace gitteh {
	Handle<Object> GetModule();

	static inline Handle<Value> CreateGitError() {  
		const git_error *err = giterr_last();
		Handle<Object> errObj = Handle<Object>::Cast(Exception::Error(
				String::New(err->message)));
		errObj->Set(String::New("code"), Integer::New(err->klass));
		return errObj;
	}

	static inline Handle<Value> ThrowGitError() {
			return ThrowException(CreateGitError());
	}

	template<typename T>
	static inline T* GetBaton(uv_work_t *req) {
			return static_cast<T*>(req->data);
	}

	/**
	Invokes provided callback with given parameters, handles catching user-land
	exceptions and propagating them to top of Node's event loop
	*/
	static inline void FireCallback(Handle<Function> callback, int argc, 
		Handle<Value> argv[]) {
		TryCatch tryCatch;
		callback->Call(Context::GetCurrent()->Global(), argc, argv);
		if(tryCatch.HasCaught()) {
			 FatalException(tryCatch);
		}
	}

	/**
		Examines return of a libgit2 call. If it's in error state, grab error object
		and hand it off to ref provided.
	*/
	static inline int LibCall(int result, const git_error **err) {
		if(result != GIT_OK) {
			*err = giterr_last();
			return 0;
		}
		return 1;
	}

	static inline void AsyncLibCall(int result, Baton *baton) {
		const git_error *err;
		if(!LibCall(result, &err)) {
			baton->setError(err);
		}
	}

	static inline void ImmutableSet(Handle<Object> o, Handle<Value> k, Handle<Value> v) {
		o->Set(k, v, (PropertyAttribute)(ReadOnly | DontDelete));
	}

	static inline string GitObjectTypeToString(git_otype type) {
		switch(type) {
			case GIT_OBJ_COMMIT: {
				return string("commit");
			}
			case GIT_OBJ_TREE: {
				return string("tree");
			}
			case GIT_OBJ_BLOB: {
				return string("blob");
			}
			case GIT_OBJ_TAG: {
				return string("tag");
			}
			default: {
				assert(0);
			}
		}
	}

} // namespace gitteh

namespace cvv8 {
	template<>
	struct NativeToJS<git_oid> {
		Handle<Value> operator() (git_oid const *oid) const {
			char oidStr[40];
			git_oid_fmt(oidStr, oid);
			return CastToJS(oidStr);
		}
	};

	template<>
	struct JSToNative<git_oid> {
		typedef git_oid ResultType;
		ResultType operator() (Handle<Value> const &h) const {
			git_oid id;
			memset(&id, 0, sizeof(git_oid));

			string idStr = CastFromJS<string>(h);
			git_oid_fromstr(&id, idStr.c_str());
			return id;
		}
	};
}

#endif // GITTEH_H
