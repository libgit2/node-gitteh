#ifndef GITTEH_H
#define GITTEH_H

#include <v8.h>
#include <node.h>
#include <git2.h>
#include <node_object_wrap.h>
#include <iostream>
#include <string.h>

using namespace v8;
using namespace node;

// The following macros were ripped from node-gd. thanks taggon!
#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                             String::New("Expected " #N " arguments")));

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
  (NAME)->Set(String::New("time"), Date::New(static_cast<double>((SRC)->when.time)*1000));


#define THROW_ERROR(errorStr) 											\
	return ThrowException(Exception::Error(String::New(errorStr)));

#define THROW_GIT_ERROR(errorStr, errorCode)							\
	return ThrowException(ThrowGitError(String::New(errorStr), errorCode));


namespace gitteh {

static inline Handle<Value> ThrowGitError(Handle<String> message, int gitErrorCode) {
	HandleScope scope;

	Handle<Object> error = Handle<Object>::Cast(Exception::Error(message));
	error->Set(String::New("gitError"), Integer::New(gitErrorCode));
	error->Set(String::New("gitErrorStr"), String::New(git_strerror(gitErrorCode)));

	return scope.Close(error);
}

} // namespace gitteh
#endif // GITTEH_H
