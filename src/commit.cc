#include "commit.h"

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
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

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theCommit);

	Commit *commit = new Commit();
	commit->commit_ = (git_commit*)theCommit->Value();

	commit->Wrap(args.This());
	commit->MakeWeak();

	return args.This();
}

Handle<Value> Commit::IdGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

		Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
		const char* oidStr = git_oid_allocfmt(git_commit_id(commit->commit_));

		return scope.Close(String::New(oidStr));
}

Handle<Value> Commit::MessageGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
	const char* message = git_commit_message(commit->commit_);

	return scope.Close(String::New(message));
}

Handle<Value> Commit::MessageShortGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
	const char* message = git_commit_message_short(commit->commit_);

	return scope.Close(String::New(message));
}

Handle<Value> Commit::TimeGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
	time_t time = git_commit_time(commit->commit_);
	return scope.Close(Date::New(static_cast<double>(time)*1000));
}

Handle<Value> Commit::AuthorGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
	const git_signature *author;
	author = git_commit_author(commit->commit_);
	CREATE_PERSON_OBJ(authorObj, author);

	return scope.Close(authorObj);
}

Handle<Value> Commit::CommitterGetter(Local<String> property, const AccessorInfo& info) {
	HandleScope scope;

	Commit *commit = ObjectWrap::Unwrap<Commit>(info.This());
	const git_signature *committer;
	committer = git_commit_committer(commit->commit_);
	CREATE_PERSON_OBJ(committerObj, committer);

	return scope.Close(committerObj);
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
}
