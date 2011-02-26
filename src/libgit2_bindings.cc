#include <v8.h>
#include <node.h>
#include <node_events.h>
#include <git2.h>

using namespace v8;
using namespace node;

class Repository : public EventEmitter {
public:
	static void Init(Handle<Object> target) {
		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		t->Inherit(EventEmitter::constructor_template);
    	t->InstanceTemplate()->SetInternalFieldCount(1);

		

		target->Set(String::New("Repository"), t->GetFunction());
	}
protected:
	static Handle<Value> New(const Arguments& args) {
		HandleScope scope;
		
		if (args.Length() == 0 || !args[0]->IsString()) {
			return ThrowException(Exception::Error(String::New("Please provide a repo path.")));
    	}
		String::Utf8Value path(args[0]->ToString());

		Repository *repo = new Repository(*path);
		repo->Wrap(args.This());
		return args.This();
	}

	Repository() : EventEmitter()  {
		
	}
	
	~Repository() {
		git_repository_free(repo);
	}

	int open(char *path) {
		return git_repository_open(path);
	}

	git_repository *repo;
};

class Git2 {
public:
	static void Init(Handle<Object> target) {
		Repository::Init(target);
	}
protected:
	
};

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Git2::Init(target);
}
