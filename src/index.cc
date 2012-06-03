#include "index.h"
#include "baton.h"
#include "repository.h"

namespace gitteh {
	static Persistent<String> class_symbol;

	class IndexBaton : public Baton {
	public:
		Index *index_;
		Repository *repository_;

		IndexBaton(Index *index) : Baton(), index_(index) {
			repository_ = index->repository_;
			index_->Ref();
		}

		~IndexBaton() {
			index_->Unref();
		}
	};

	class ReadTreeBaton : public IndexBaton {
	public:
		git_oid treeId;

		ReadTreeBaton(Index *index) : IndexBaton(index)  { }
	};

	Persistent<FunctionTemplate> Index::constructor_template;

	Index::Index(Repository *repository, git_index *index) : 
			repository_(repository), index_(index) {

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

		NODE_SET_PROTOTYPE_METHOD(t, "readTree", ReadTree);
		NODE_SET_PROTOTYPE_METHOD(t, "write", Write);

		module->Set(class_symbol, constructor_template->GetFunction());
	}

	Handle<Value> Index::New(const Arguments &args) {
		HandleScope scope;
		REQ_EXT_ARG(0, repoArg);
		REQ_EXT_ARG(1, indexArg);
		Handle<Object> me = args.This();

		Repository *repository = static_cast<Repository*>(repoArg->Value());
		git_index *rawIndex = static_cast<git_index*>(indexArg->Value());
		Index *index = new Index(repository, rawIndex);
		index->Wrap(me);

		return scope.Close(me);
	}

	Handle<Value> Index::ReadTree(const Arguments &args) {
		HandleScope scope;
		Index *index = ObjectWrap::Unwrap<Index>(args.This());

		ReadTreeBaton *baton = new ReadTreeBaton(index);
		baton->treeId = CastFromJS<git_oid>(args[0]);
		baton->setCallback(args[1]);

		uv_queue_work(uv_default_loop(), &baton->req, AsyncReadTree,
				AsyncAfterReadTree);

		return Undefined();
	}

	void Index::AsyncReadTree(uv_work_t *req) {
		ReadTreeBaton *baton = GetBaton<ReadTreeBaton>(req);

		baton->repository_->lockRepository();

		git_tree *tree;
		if(AsyncLibCall(git_tree_lookup(&tree, baton->repository_->repo_,
				&baton->treeId), baton)) {
			AsyncLibCall(git_index_read_tree(baton->index_->index_,
					tree), baton);
			git_tree_free(tree);
		}

		baton->repository_->unlockRepository();
	}

	void Index::AsyncAfterReadTree(uv_work_t *req) {
		HandleScope scope;
		ReadTreeBaton *baton = GetBaton<ReadTreeBaton>(req);

		baton->defaultCallback();

		delete baton;
	}

	Handle<Value> Index::Write(const Arguments &args) {
		HandleScope scope;
		Index *index = ObjectWrap::Unwrap<Index>(args.This());
		IndexBaton *baton = new IndexBaton(index);
		baton->setCallback(args[0]);

		uv_queue_work(uv_default_loop(), &baton->req, AsyncWrite, 
				AsyncAfterWrite);

		return Undefined();
	}

	void Index::AsyncWrite(uv_work_t *req) {
		IndexBaton *baton = GetBaton<IndexBaton>(req);

		AsyncLibCall(git_index_write(baton->index_->index_), baton);
	}

	void Index::AsyncAfterWrite(uv_work_t *req) {
		HandleScope scope;
		IndexBaton *baton = GetBaton<IndexBaton>(req);

		baton->defaultCallback();

		delete baton;
	}

}; // namespace gitteh
