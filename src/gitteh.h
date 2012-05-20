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

#define SIG_TIME_PROPERTY String::NewSymbol("time")
#define SIG_OFFSET_PROPERTY String::NewSymbol("timeOffset")
#define SIG_EMAIL_PROPERTY String::NewSymbol("email")
#define SIG_NAME_PROPERTY String::NewSymbol("name")

#define HAS_CALLBACK_ARG args[args.Length()-1]->IsFunction()

#define CLEANUP_CALLBACK(CALLBACK)											\
	(CALLBACK).Dispose();													\
	(CALLBACK).Clear();

#define TRIGGER_CALLBACK()													\
	TryCatch tryCatch;													    \
	baton->callback->Call(Context::GetCurrent()->Global(),				     \
			2, callbackArgs);												\
 	if(tryCatch.HasCaught()) {												\
       FatalException(tryCatch);											\
    }

#define CHECK_PROPERTY(PROPNAME)											\
	if(args.This()->Get(PROPNAME)->IsUndefined() ||							\
				args.This()->Get(PROPNAME)->IsNull())						\
		THROW_ERROR("Property " #PROPNAME " is required.");

// The following macros were ripped from node-gd. thanks taggon!
#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                             String::New("Expected " #N " arguments")));

#define REQ_OBJ_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsObject())                     \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be an object.")));    \
  Local<Object> VAR(args[I]->ToObject());

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a string")));    \
  String::Utf8Value VAR(args[I]->ToString());

#define REQ_INT_ARG(I, VAR)                                             \
  int VAR;                                                              \
  if (args.Length() <= (I) || !args[I]->IsInt32())                      \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be an integer")));  \
  VAR = args[I]->Int32Value();

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a function")));  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_DOUBLE_ARG(I, VAR)                                          \
  double VAR;                                                           \
  if (args.Length() <= (I) || !args[I]->IsNumber())                     \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " must be a number")));    \
  VAR = args[I]->NumberValue();

#define REQ_EXT_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsExternal())                   \
    return ThrowException(Exception::TypeError(                         \
                  String::New("Argument " #I " invalid")));             \
  Local<External> VAR = Local<External>::Cast(args[I]);

#define REQ_OID_ARG(I, VAR)												\
  if (args.Length() <= (I) || !args[I]->IsString()) 					\
	return ThrowException(Exception::TypeError(							\
				  String::New("Argument " #I " invalid")));				\
  git_oid VAR;															\
  if(git_oid_fromstr(& VAR, *(String::Utf8Value(args[I]->ToString()))) != GIT_OK) \
  	return ThrowGitError();

#define CREATE_PERSON_OBJ(NAME, SRC)									\
  Local<Object> NAME = Object::New();									\
  (NAME)->Set(String::New("name"), String::New((SRC)->name));			\
  (NAME)->Set(String::New("email"), String::New((SRC)->email));			\
  (NAME)->Set(String::New("time"), NODE_UNIXTIME_V8((SRC)->when.time));	\
  (NAME)->Set(String::New("timeOffset"), Integer::New((SRC)->when.offset));


#define THROW_ERROR(errorStr) 											\
	return ThrowException(Exception::Error(String::New(errorStr)));

#define THROW_GIT_ERROR(errorStr, errorCode)							\
	return scope.Close(ThrowException(CreateGitError(String::New(errorStr), errorCode)));

#define THROW_GIT_ERROR2(errorStr)              \
  return scope.Close(ThrowException(CreateGitError(String::New(errorStr), giterr_last())));

#define LOAD_OID_ARG(I, VAR)												\
  if (args.Length() <= (I) || !args[I]->IsString()) 						\
	return ThrowException(Exception::TypeError(								\
				  String::New("Argument " #I " invalid")));					\
  if(git_oid_fromstr(&VAR, *(String::Utf8Value(args[I]->ToString()))) != GIT_OK) \
  	return scope.Close(ThrowGitError());

namespace gitteh {

/*static inline Handle<Value> CreateGitError(Handle<String> message, const git_error* error) {
	HandleScope scope;

	Handle<Object> errorObj = Handle<Object>::Cast(Exception::Error(message));
	errorObj->Set(String::New("gitError"), Integer::New(error->klass));
	errorObj->Set(String::New("gitErrorStr"), String::New(error->message));

	return scope.Close(errorObj);
}*/

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
      memcpy(&baton->error, err, sizeof(git_error));
  }
}

} // namespace gitteh
#endif // GITTEH_H
