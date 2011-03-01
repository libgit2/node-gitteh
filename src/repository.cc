#include "repository.h"
#include "commit.h"
#include "tree.h"
#include "odb.h"
#include "index.h"

using namespace std;

static void StubWeakCallback(Persistent<Value> val, void *data) {
	// TODO: should we actually be doing anything here? node's ObjectWrap deletes the object, but as far as I can tell, the dtor
	// for my weak ref'd stuff is getting called, which means memory is being freed... Unless I'm completely retarded?
}

Persistent<FunctionTemplate> Repository::constructor_template;

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Repository"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "getObjectDatabase", GetODB);
	NODE_SET_PROTOTYPE_METHOD(t, "getCommit", GetCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);

	t->InstanceTemplate()->SetAccessor(String::New("index"), IndexGetter);

	target->Set(String::New("Repository"), t->GetFunction());
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_STR_ARG(0, path);

	Repository *repo = new Repository();

	if(int result = git_repository_open(&repo->repo_, *path) != GIT_SUCCESS) {
		Handle<Value> ex = Exception::Error(String::New("Git error."));
		return ThrowException(ex);
	}

	repo->Wrap(args.This());
	repo->MakeWeak();

	return args.This();
}

Handle<Value> Repository::GetODB(const Arguments& args) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	git_odb *odb = git_repository_database(repo->repo_);

	Local<Value> arg = External::New(odb);
	Persistent<Object> result(ObjectDatabase::constructor_template->GetFunction()->NewInstance(1, &arg));
	return scope.Close(result);
}

Handle<Value> Repository::GetCommit(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_OID_ARG(0, commitOid);

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	const char* oidStr = git_oid_allocfmt(&commitOid);

	git_commit* commit;
	if(git_commit_lookup(&commit, repo->repo_, &commitOid) != GIT_SUCCESS) {
		// TODO: error code handling.
		return scope.Close(Null());
	}

	Commit *commitObject = repo->wrapCommit(commit);
	return scope.Close(commitObject->handle_);
}

Handle<Value> Repository::GetTree(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_OID_ARG(0, commitOid);

	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	git_tree *tree;
	if(git_tree_lookup(&tree, repo->repo_, &commitOid) != GIT_SUCCESS) {
		return scope.Close(Null());
	}

	Tree *treeObject = repo->wrapTree(tree);
	return scope.Close(treeObject->handle_);
}

Handle<Value> Repository::IndexGetter(Local<String>, const AccessorInfo& info) {
	HandleScope scope;

	Repository *repo = ObjectWrap::Unwrap<Repository>(info.This());
	if(repo->index_ == NULL) {
		git_index *index;
		git_repository_index(&index, repo->repo_);
		Handle<Value> arg = External::New(index);
		Handle<Object> instance = Index::constructor_template->GetFunction()->NewInstance(1, &arg);
		repo->index_ = ObjectWrap::Unwrap<Index>(instance);
	}

	return repo->index_->handle_;
}

Repository::~Repository() {
	close();
}

void Repository::close() {
	if(repo_) {
		git_repository_free(repo_);
		repo_ = NULL;
	}
}

Commit *Repository::wrapCommit(git_commit *commit) {
	Commit *commitObject;
	if(commitStore_.getObjectFor(commit, &commitObject)) {
		// Commit needs to know who it's daddy is.
		commitObject->repository_ = this;
	}

	return commitObject;
}

Tree *Repository::wrapTree(git_tree *tree) {
	Tree *treeObject;
	if(treeStore_.getObjectFor(tree, &treeObject)) {
		treeObject->repository_ = this;
	}

	return treeObject;
}
