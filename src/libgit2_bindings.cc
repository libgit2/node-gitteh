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

#define COMMIT_ID_SYMBOL String::NewSymbol("id")
#define COMMIT_MESSAGE_SHORT_SYMBOL String::NewSymbol("messageShort")
#define COMMIT_MESSAGE_SYMBOL String::NewSymbol("message")
#define COMMIT_TIME_SYMBOL String::NewSymbol("time")
#define COMMIT_AUTHOR_SYMBOL String::NewSymbol("author")
#define COMMIT_COMMITTER_SYMBOL String::NewSymbol("committer")
#define COMMIT_TREE_SYMBOL String::NewSymbol("tree")

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

#define TREE_ENTRY_NAME_SYMBOL String::NewSymbol("name")


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

#define CREATE_PERSON_OBJ(NAME, SRC)									\
  Local<Object> NAME = Object::New();									\
  (NAME)->Set(String::New("name"), String::New((SRC)->name));			\
  (NAME)->Set(String::New("email"), String::New((SRC)->email));

class TreeEntry : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	
	static void Init(Handle<Object> target) {
		HandleScope scope;
		
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::New("TreeEntry"));
		t->InstanceTemplate()->SetInternalFieldCount(1);
		
		t->PrototypeTemplate()->SetAccessor(TREE_ENTRY_NAME_SYMBOL, NameGetter);
	}
protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_EXT_ARG(0, theEntry);
		
		TreeEntry *entry = new TreeEntry();
		entry->entry_ = (git_tree_entry*)theEntry->Value();

		entry->Wrap(args.This());
		entry->MakeWeak();

		return args.This();
	}
	
	static Handle<Value> NameGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		TreeEntry *entry = ObjectWrap::Unwrap<TreeEntry>(info.This());
		const char* fileName = git_tree_entry_name(entry->entry_);
		
		return scope.Close(String::New(fileName));
	}

	git_tree_entry *entry_;
};

class Tree : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	
	static void Init(Handle<Object> target) {
		HandleScope scope;
		
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::New("Tree"));
		t->InstanceTemplate()->SetInternalFieldCount(1);
		
		t->PrototypeTemplate()->SetAccessor(TREE_ID_SYMBOL, IdGetter);
		t->PrototypeTemplate()->SetAccessor(TREE_LENGTH_SYMBOL, LengthGetter);
		t->PrototypeTemplate()->SetIndexedPropertyHandler(IndexHandler);
	}
protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_EXT_ARG(0, theTree);

		Tree *tree = new Tree();
		tree->tree_ = (git_tree*)theTree->Value();
		
		tree->Wrap(args.This());
		tree->MakeWeak();
		
		return args.This();
	}
	
	static Handle<Value> IdGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
		const char* oidStr = git_oid_allocfmt(git_tree_id(tree->tree_));		
		
		return scope.Close(String::New(oidStr));
	}
	
	static Handle<Value> LengthGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
		size_t entryCount = git_tree_entrycount(tree->tree_);		
		
		return scope.Close(Integer::New(entryCount));
	}

	static Handle<Value> IndexHandler(uint32_t index, const AccessorInfo& info) {
		HandleScope scope;
		
		Tree *tree = ObjectWrap::Unwrap<Tree>(info.This());
		size_t entryCount = git_tree_entrycount(tree->tree_);		
		
		if(index > (entryCount-1)) {
			return ThrowException(Exception::Error(String::New("Tree entry index is out of range.")));
		}
		
		git_tree_entry *entry = git_tree_entry_byindex(tree->tree_, index);
		
		Local<Value> arg = External::New(entry);
		Persistent<Object> result(TreeEntry::constructor_template->GetFunction()->NewInstance(1, &arg));
		return scope.Close(result);
	}

	git_tree *tree_;
};

class Commit : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	
	static void Init(Handle<Object> target) {
		HandleScope scope;
		
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(String::New("Commit"));
		t->InstanceTemplate()->SetInternalFieldCount(1);

		t->PrototypeTemplate()->SetAccessor(COMMIT_ID_SYMBOL, IdGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_MESSAGE_SYMBOL, MessageGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_MESSAGE_SHORT_SYMBOL, MessageShortGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_TIME_SYMBOL, TimeGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_AUTHOR_SYMBOL, AuthorGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_COMMITTER_SYMBOL, CommitterGetter);
		t->PrototypeTemplate()->SetAccessor(COMMIT_TREE_SYMBOL, TreeGetter);
	}

protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
	
		REQ_ARGS(1);
		REQ_EXT_ARG(0, theCommit);
	
		Commit *commit = new Commit();
		commit->commit_ = (git_commit*)theCommit->Value();
	
		commit->Wrap(args.This());
		commit->MakeWeak();

		return args.This();
	}

	static Handle<Value> IdGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const char* oidStr = git_oid_allocfmt(git_commit_id(commit->commit_));		
		
		return scope.Close(String::New(oidStr));
	}
	
	static Handle<Value> MessageGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const char* message = git_commit_message(commit->commit_);
		
		return scope.Close(String::New(message));
	}

	static Handle<Value> MessageShortGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const char* message = git_commit_message_short(commit->commit_);
		
		return scope.Close(String::New(message));
	}

	static Handle<Value> TimeGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		time_t time = git_commit_time(commit->commit_);
		return scope.Close(Date::New(static_cast<double>(time)*1000));
	}
	
	static Handle<Value> AuthorGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const git_signature *author;
		author = git_commit_author(commit->commit_);
		CREATE_PERSON_OBJ(authorObj, author);

		return scope.Close(authorObj);
	}
	
	static Handle<Value> CommitterGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;
		
		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const git_signature *committer;
		committer = git_commit_committer(commit->commit_);
		CREATE_PERSON_OBJ(committerObj, committer);

		return scope.Close(committerObj);
	}
	
	static Handle<Value> TreeGetter(Local<String> property, const AccessorInfo& info) {
		HandleScope scope;


		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		
		const git_tree *tree = git_commit_tree(commit->commit_);

		Local<Value> arg = External::New((void*)tree);
		Persistent<Object> result(Tree::constructor_template->GetFunction()->NewInstance(1, &arg));
		return scope.Close(result);
	}

	git_commit *commit_;
};

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
    	NODE_SET_PROTOTYPE_METHOD(t, "getCommit", GetCommit);

		target->Set(String::New("Repository"), t->GetFunction());
	}

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
	
	static Handle<Value> GetCommit(const Arguments& args) {
		HandleScope scope;
		
		REQ_ARGS(1);
		REQ_OID_ARG(0, commitOid);

		Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
		
		git_commit* commit;
		if(git_commit_lookup(&commit, repo->repo_, &commitOid) != GIT_SUCCESS) {
			// TODO: error code handling.
			return Null();
		}
		
		Local<Value> arg = External::New(commit);
		Persistent<Object> result(Commit::constructor_template->GetFunction()->NewInstance(1, &arg));
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

	git_repository *repo_;
};

class Git2 {
public:
	static void Init(Handle<Object> target) {
		Repository::Init(target);
		RawObject::Init(target);
		ObjectDatabase::Init(target);
		Commit::Init(target);
		Tree::Init(target);
		TreeEntry::Init(target);
	}
};

Persistent<FunctionTemplate> ObjectDatabase::constructor_template;
Persistent<FunctionTemplate> RawObject::constructor_template;
Persistent<FunctionTemplate> Commit::constructor_template;
Persistent<FunctionTemplate> Tree::constructor_template;
Persistent<FunctionTemplate> TreeEntry::constructor_template;

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Git2::Init(target);
}
