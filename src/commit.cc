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

#include "commit.h"
#include "repository.h"
#include <time.h>
#include <stdlib.h>
#include "signature.h"

static Persistent<String> commit_class_symbol;
static Persistent<String> id_symbol;
static Persistent<String> message_symbol;
static Persistent<String> message_encoding_symbol;
static Persistent<String> author_symbol;
static Persistent<String> committer_symbol;
static Persistent<String> tree_symbol;
static Persistent<String> tree_id_symbol;
static Persistent<String> parents_symbol;

namespace gitteh {

Persistent<FunctionTemplate> Commit::constructor_template;

void Commit::Init(Handle<Object> target) {
	HandleScope scope;

	commit_class_symbol = NODE_PSYMBOL("Commit");
	id_symbol = NODE_PSYMBOL("id");
	message_symbol = NODE_PSYMBOL("message");
	message_encoding_symbol = NODE_PSYMBOL("messageEncoding");
	author_symbol = NODE_PSYMBOL("author");
	committer_symbol = NODE_PSYMBOL("committer");
	tree_symbol = NODE_PSYMBOL("tree");
	tree_id_symbol = NODE_PSYMBOL("treeId");
	parents_symbol = NODE_PSYMBOL("parents");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(commit_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	// NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	target->Set(commit_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Commit::New(const Arguments& args) {
	HandleScope scope;
	REQ_EXT_ARG(0, commitArg);

	Commit *commitObj = static_cast<Commit*>(commitArg->Value());
	commitObj->Wrap(args.This());

	Handle<Object> me = args.This();

	git_commit *commit = commitObj->commit_;
	char oidStr[40];
	const git_oid *oid;

	oid = git_commit_id(commit);
	git_oid_fmt(oidStr, oid);
	ImmutableSet(me, id_symbol, CastToJS(oidStr));

	oid = git_commit_tree_oid(commit);
	git_oid_fmt(oidStr, oid);
	ImmutableSet(me, tree_id_symbol, CastToJS(oidStr));

	ImmutableSet(me, message_symbol, CastToJS(git_commit_message(commit)));
	const char *encoding = git_commit_message_encoding(commit);
	if(encoding) ImmutableSet(me, message_encoding_symbol, CastToJS(encoding));

	// TODO: immutable me bro.
	Handle<Array> parents = Array::New();
	int parentCount = git_commit_parentcount(commit);
	for(int i = 0; i < parentCount; i++) {
		oid = git_commit_parent_oid(commit, i);
		git_oid_fmt(oidStr, oid);
		parents->Set(i, CastToJS(oidStr));
	}
	me->Set(parents_symbol, parents);

	ImmutableSet(me, author_symbol, CreateSignature(git_commit_author(commit)));
	ImmutableSet(me, committer_symbol, CreateSignature(git_commit_committer(commit)));

	return args.This();
}

Commit::Commit(git_commit *commit) : GitObject((git_object*)commit) {
	commit_ = commit;
}

Commit::~Commit() {
}

}; // namespace gitteh
