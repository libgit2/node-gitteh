#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <node_buffer.h>
#include <git2.h>
#include <iostream>
#include <string.h>

using namespace std;
using namespace v8;
using namespace node;

#define RAWOBJ_TYPE_SYMBOL String::NewSymbol("type")
#define RAWOBJ_DATA_SYMBOL String::NewSymbol("data")

// The following macros were ripped from node-gd. thanks taggon!
#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowException(Exception::TypeError(                         \
                             String::New("Expected " #N "arguments"))); 

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

class RawObject : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	
	static void Init(Handle<Object> target) {
		HandleScope scope;
		
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::NewSymbol("RawObject"));
		t->InstanceTemplate()->SetInternalFieldCount(1);
		
		t->PrototypeTemplate()->SetAccessor(RAWOBJ_TYPE_SYMBOL, TypeGetter);
		t->PrototypeTemplate()->SetAccessor(RAWOBJ_DATA_SYMBOL, DataGetter);
	}
protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_EXT_ARG(0, theObj);
		
		RawObject *obj = new RawObject();
		obj->obj_ = *((git_rawobj*)theObj->Value());

		obj->Wrap(args.This());
		obj->MakeWeak();
		
		return args.This();
	}
	
	static Handle<Value> TypeGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		RawObject *rawObject = ObjectWrap::Unwrap<RawObject>(info.This());

		Local<Number> type = Integer::New(rawObject->obj_.type);
		return scope.Close(type);
	}
	
	static Handle<Value> DataGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		RawObject *rawObject = ObjectWrap::Unwrap<RawObject>(info.This());
		Buffer *buf = Buffer::New(rawObject->obj_.len);
		memcpy(Buffer::Data(buf), rawObject->obj_.data, rawObject->obj_.len);
		
		return scope.Close(buf->handle_);
	}

	git_rawobj obj_;
};

class ObjectDatabase : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;

	static void Init(Handle<Object> target) {
		HandleScope scope;

		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::NewSymbol("ObjectDatabase"));
    	t->InstanceTemplate()->SetInternalFieldCount(1);

    	NODE_SET_PROTOTYPE_METHOD(t, "read", Read);
	}
protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;

		REQ_EXT_ARG(0, theOdb);

		ObjectDatabase *odb = new ObjectDatabase();
		odb->odb_ = (git_odb*)theOdb->Value();
		
		odb->Wrap(args.This());
		odb->MakeWeak();
		
		return args.This();
	}
	
	static Handle<Value> Read(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_OID_ARG(0, oid);
		
		ObjectDatabase *odb = ObjectWrap::Unwrap<ObjectDatabase>(args.This());

		git_rawobj obj;
		if(git_odb_read(&obj, odb->odb_, &oid) == GIT_ENOTFOUND) {
			return Null();
		}

		Local<Value> arg = External::New(&obj);
		Persistent<Object> result(RawObject::constructor_template->GetFunction()->NewInstance(1, &arg));
		return scope.Close(result);
	}
	
	ObjectDatabase() {
	}

	git_odb *odb_;
};

class Repository : public EventEmitter {
public:
	static void Init(Handle<Object> target) {
		HandleScope scope;

		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		t->Inherit(EventEmitter::constructor_template);
    	t->InstanceTemplate()->SetInternalFieldCount(1);
    	
    	NODE_SET_PROTOTYPE_METHOD(t, "getObjectDatabase", GetODB);

		target->Set(String::New("Repository"), t->GetFunction());
	}
	git_repository *repo_;

protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_STR_ARG(0, path);

		Repository *repo = new Repository();

		if(int result = repo->open(*path) != GIT_SUCCESS) {
			Handle<Value> ex = Exception::Error(String::New("Git error."));
			return ThrowException(ex);
		}
		
		repo->Wrap(args.This());
		repo->MakeWeak();

		return args.This();
	}
	
	static Handle<Value> GetODB(const Arguments& args) {
		HandleScope scope;

		Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
		git_odb *odb = repo->getOdb();
		
		Local<Value> arg = External::New(odb);
		Persistent<Object> result(ObjectDatabase::constructor_template->GetFunction()->NewInstance(1, &arg));
		return scope.Close(result);
	}

	Repository() : EventEmitter()  {
		
	}
	
	~Repository() {
		close();
	}

	int open(char *path) {
		return git_repository_open(&repo_, path);
	}
	
	git_odb *getOdb() {
		return git_repository_database(repo_);
	}

	void close() {
		if(repo_) {
			git_repository_free(repo_);
			repo_ = NULL;
		}
	}
};

class Git2 {
public:
	static void Init(Handle<Object> target) {
		Repository::Init(target);
		RawObject::Init(target);
		ObjectDatabase::Init(target);
	}
};

Persistent<FunctionTemplate> ObjectDatabase::constructor_template;
Persistent<FunctionTemplate> RawObject::constructor_template;

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Git2::Init(target);
}
