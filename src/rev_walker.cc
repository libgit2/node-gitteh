/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "rev_walker.h"
#include "commit.h"
#include "repository.h"

namespace gitteh {

Persistent<FunctionTemplate> RevWalker::constructor_template;

void RevWalker::Init(Handle<Object> target) {
	HandleScope scope;

	target->Set(String::New("SORT_NONE"), Integer::New(GIT_SORT_NONE));
	target->Set(String::New("SORT_TOPOLOGICAL"), Integer::New(GIT_SORT_TOPOLOGICAL));
	target->Set(String::New("SORT_TIME"), Integer::New(GIT_SORT_TIME));
	target->Set(String::New("SORT_REVERSE"), Integer::New(GIT_SORT_REVERSE));

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("RevWalker"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "push", Push);
	NODE_SET_PROTOTYPE_METHOD(t, "hide", Hide);
	NODE_SET_PROTOTYPE_METHOD(t, "next", Next);
	NODE_SET_PROTOTYPE_METHOD(t, "sort", Sort);
	NODE_SET_PROTOTYPE_METHOD(t, "reset", Reset);
}

Handle<Value> RevWalker::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theWalker);

	RevWalker *walker = new RevWalker();
	walker->walker_ = static_cast<git_revwalk *>(theWalker->Value());

	walker->Wrap(args.This());

	return args.This();
}

Handle<Value> RevWalker::Push(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit;
	if(args[0]->IsString()) {
		REQ_OID_ARG(0, commitOid);
		int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);
		if(result != GIT_SUCCESS)
			THROW_GIT_ERROR("Commit not found.", result);
	}
	else {
		// Commit object.
		if(!args[0]->IsObject()) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Handle<Object> commitArg = Handle<Object>::Cast(args[0]);
		if(!Commit::constructor_template->HasInstance(commitArg)) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
		commit = commitObject->commit_;
		//return ThrowException(Exception::Error(String::New("Passing commit object is not supported yet.")));
	}

	// Get the commit for this oid.
	int res = git_revwalk_push(walker->walker_, git_commit_id(commit));
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't push commit onto walker.", res);

	return scope.Close(Undefined());
}

Handle<Value> RevWalker::Hide(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_commit *commit;
	if(args[0]->IsString()) {
		REQ_OID_ARG(0, commitOid);
		int result = git_commit_lookup(&commit, walker->repo_->repo_, &commitOid);

		if(result != GIT_SUCCESS) {
			return ThrowException(Exception::Error(String::New("Commit not found.")));
		}
	}
	else {
		// Commit object.
		if(!args[0]->IsObject()) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Handle<Object> commitArg = Handle<Object>::Cast(args[0]);
		if(!Commit::constructor_template->HasInstance(commitArg)) {
			return ThrowException(Exception::Error(String::New("Invalid commit object.")));
		}

		Commit *commitObject = ObjectWrap::Unwrap<Commit>(commitArg);
		commit = commitObject->commit_;
	}
	
	int res = git_revwalk_hide(walker->walker_, git_commit_id(commit));
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't hide commit.", res);

	return scope.Close(Undefined());
}

Handle<Value> RevWalker::Next(const Arguments& args) {
	HandleScope scope;

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	git_oid id;
	int result = git_revwalk_next(&id, walker->walker_);
	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't get next commit.", result);
	}

	git_commit *commit;
	result = git_commit_lookup(&commit, walker->repo_->repo_, &id);
	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't get next commit.", result);
	}

	// TODO:
//	Commit *commitObject = walker->repo_->wrapCommit(commit);
	//return scope.Close(commitObject->handle_);
	return Undefined();
}

Handle<Value> RevWalker::Sort(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_INT_ARG(0, sorting);

	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());

	int result = git_revwalk_sorting(walker->walker_, sorting);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't sort rev walker.", result);

	return Undefined();
}

Handle<Value> RevWalker::Reset(const Arguments& args) {
	HandleScope scope;
	
	RevWalker *walker = ObjectWrap::Unwrap<RevWalker>(args.This());
	git_revwalk_reset(walker->walker_);

	return scope.Close(Undefined());
}

RevWalker::~RevWalker() {
	git_revwalk_free(walker_);
}

} // namespace gitteh
