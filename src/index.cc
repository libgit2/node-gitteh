#include "index.h"

namespace gitteh {
	static Persistent<String> class_symbol;

	Persistent<FunctionTemplate> Index::constructor_template;

	Index::Index(git_index *index) : index_(index) {

	}

	Index::~Index() {

	}

	void Index::Init(Handle<Object> module) {
		HandleScope scope;

		class_symbol = NODE_PSYMBOL("NativeIndex");

		Local<FunctionTemplate> t = FunctionTemplate::New(New);
		constructor_template = Persistent<FunctionTemplate>::New(t);
		constructor_template->SetClassName(class_symbol);
		t->InstanceTemplate()->SetInternalFieldCount(1);

		module->Set(class_symbol, constructor_template->GetFunction());
	}

	Handle<Value> Index::New(const Arguments &args) {
		HandleScope scope;
		REQ_EXT_ARG(0, indexArg);
		Handle<Object> me = args.This();

		git_index *rawIndex = static_cast<git_index*>(indexArg->Value());
		Index *index = new Index(rawIndex);
		index->Wrap(me);

		return scope.Close(me);
	}
}; // namespace gitteh
