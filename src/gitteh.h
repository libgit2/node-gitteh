#ifndef GITTEH_H
#define GITTEH_H

#include <v8.h>
#include <node.h>
#include <git2.h>
#include <node_object_wrap.h>
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>

#include "thread.h"

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
	TryCatch tryCatch;														\
	reqData->callback->Call(Context::GetCurrent()->Global(),				\
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
  if(git_oid_mkstr(& VAR, *(String::Utf8Value(args[I]->ToString()))) == GIT_ENOTOID) \
  	return ThrowException(Exception::TypeError(							\
  				  String::New("Argument " #I " is not an oid")));

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

#define LOAD_OID_ARG(I, VAR)												\
  if (args.Length() <= (I) || !args[I]->IsString()) 						\
	return ThrowException(Exception::TypeError(								\
				  String::New("Argument " #I " invalid")));					\
  if(git_oid_mkstr(&VAR, *(String::Utf8Value(args[I]->ToString()))) == GIT_ENOTOID) \
  	return ThrowException(Exception::TypeError(								\
  				  String::New("Argument " #I " is not an oid")));

namespace gitteh {

static inline Handle<Value> CreateGitError(Handle<String> message, int gitErrorCode) {
	HandleScope scope;

	Handle<Object> error = Handle<Object>::Cast(Exception::Error(message));
	error->Set(String::New("gitError"), Integer::New(gitErrorCode));
	error->Set(String::New("gitErrorStr"), String::New(git_strerror(gitErrorCode)));

	return scope.Close(error);
}

} // namespace gitteh
#endif // GITTEH_H
