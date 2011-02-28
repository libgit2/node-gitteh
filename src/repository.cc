#include "repository.h"
#include "commit.h"
#include "odb.h"

using namespace std;

static void StubWeakCallback(Persistent<Value> val, void* data) {
	// TODO: should we actually be doing anything here? node's ObjectWrap deletes the object, but as far as I can tell, the dtor
	// for my weak ref'd stuff is getting called, which means memory is being freed... Unless I'm completely retarded?
}

Persistent<FunctionTemplate> Repository::constructor_template;

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Repository"));
	t->InstanceTemplate()->SetInternalFieldCount(3);

	NODE_SET_PROTOTYPE_METHOD(t, "getObjectDatabase", GetODB);
	NODE_SET_PROTOTYPE_METHOD(t, "getCommit", GetCommit);

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

	// Create our backing store.
	Persistent<Object> commitStore = Persistent<Object>::New(Object::New());
	commitStore.MakeWeak(&commitStore, StubWeakCallback);
	args.This()->SetInternalField(REPO_INTERNAL_FIELD_COMMIT_STORE, commitStore);

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
	Local<Object> commitStore = Local<Object>::Cast(args.This()->GetInternalField(REPO_INTERNAL_FIELD_COMMIT_STORE));
	const char* oidStr = git_oid_allocfmt(&commitOid);

	Local<Value> possibleCommit = commitStore->Get(String::New(oidStr));

	if(!possibleCommit->IsUndefined()) {
		return scope.Close(commitStore->Get(String::New(oidStr)));
	}

	git_commit* commit;
	if(git_commit_lookup(&commit, repo->repo_, &commitOid) != GIT_SUCCESS) {
		// TODO: error code handling.
		return Null();
	}

	Local<Value> arg = External::New(commit);
	Persistent<Object> result(Commit::constructor_template->GetFunction()->NewInstance(1, &arg));
	result.MakeWeak(&arg, StubWeakCallback);

	commitStore->Set(String::NewSymbol(oidStr), result);

	return scope.Close(result);
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
