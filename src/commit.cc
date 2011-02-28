#include "commit.h"

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Commit"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	/*t->PrototypeTemplate()->SetAccessor(COMMIT_ID_SYMBOL, IdGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_MESSAGE_SYMBOL, MessageGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_MESSAGE_SHORT_SYMBOL, MessageShortGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_TIME_SYMBOL, TimeGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_AUTHOR_SYMBOL, AuthorGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_COMMITTER_SYMBOL, CommitterGetter);
	t->PrototypeTemplate()->SetAccessor(COMMIT_TREE_SYMBOL, TreeGetter);*/
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);
	REQ_EXT_ARG(1, theRepo);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();
	commit->repo_ = (Repository*)theRepo->Value();

	// Setup some basic info about this commit.
	const char* oidStr = git_oid_allocfmt(git_commit_id(commit->commit_));
	args.This()->Set(String::New("id"), String::New(oidStr));

	const char* message = git_commit_message(commit->commit_);
	args.This()->Set(String::New("message"), String::New(message));

	const char* shortMessage = git_commit_message_short(commit->commit_);
	args.This()->Set(String::New("shortMessage"), String::New(shortMessage));

	time_t time = git_commit_time(commit->commit_);
	args.This()->Set(String::New("time"), Date::New(static_cast<double>(time)*1000));

	const git_signature *author;
	author = git_commit_author(commit->commit_);
	CREATE_PERSON_OBJ(authorObj, author);
	args.This()->Set(String::New("author"), authorObj);

	const git_signature *committer;
	committer = git_commit_committer(commit->commit_);
	CREATE_PERSON_OBJ(committerObj, committer);
	args.This()->Set(String::New("committer"), committerObj);

	commit->Wrap(args.This());
	return args.This();
}

Handle<Value> Commit::TreeGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());

	const git_tree *tree = git_commit_tree(commit->commit_);

	Local<Value> arg = External::New((void*)tree);
	Persistent<Object> result(Tree::constructor_template->GetFunction()->NewInstance(1, &arg));
	return scope.Close(result);
}

Commit::~Commit() {
	// TODO: don't think we ever need to free commits as they're handled by the repo, even newly created ones
	// (I think), probably need to look into this.
	repo_->notifyCommitDeath(commit_);
}
