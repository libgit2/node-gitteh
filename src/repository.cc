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

#include "repository.h"
#include "git_object.h"
#include <sys/time.h>

#include "commit.h"
#include "tree.h"

namespace gitteh {
static Persistent<String> repo_class_symbol;
static Persistent<String> path_symbol;
static Persistent<String> bare_symbol;

static Persistent<String> git_dir_symbol;
static Persistent<String> object_dir_symbol;
static Persistent<String> index_file_symbol;
static Persistent<String> work_dir_symbol;

class OpenRepoBaton : public Baton {
public:
	string path	;
	git_repository *repo;

	OpenRepoBaton(string path) : Baton(), path(path) {}	;
};

class InitRepoBaton : public Baton {
public:
	string path	;
	bool bare;
	git_repository *repo;
};

class RepositoryBaton : public Baton {
public:
	Repository *repo;

	RepositoryBaton(Repository *_repo) : Baton(), repo(_repo) {
		repo->Ref();
	};

	~RepositoryBaton() {
		repo->Unref();
	}
};

class ExistsBaton : public RepositoryBaton {
public:
	git_oid oid;
	bool exists;

	ExistsBaton(Repository *r, git_oid oid) : RepositoryBaton(r), oid(oid) {}
};

class GetObjectBaton : public RepositoryBaton {
public:
	git_oid oid;
	git_object *object;
	GetObjectBaton(Repository *r, git_oid oid) : RepositoryBaton(r), oid(oid) {}
};

Persistent<FunctionTemplate> Repository::constructor_template;

Repository::Repository() : cache_(this) {
	CREATE_MUTEX(gitLock_);

	odb_ = NULL;
	repo_ = NULL;
}

Repository::~Repository() {
	std::cout << "~Repository" << std::endl;
	if(odb_) {
		git_odb_free(odb_);
		odb_ = NULL;
	}

	if(repo_) {
		git_repository_free(repo_);
		repo_ = NULL;
	}

	DESTROY_MUTEX(gitLock_);

	/*delete commitCache_;
	delete referenceCache_;
	delete treeCache_;
	delete tagCache_;
	delete blobCache_;

	close();*/
}

void Repository::adopt(GitObject *obj) {
	Ref();
}

void Repository::disown(GitObject *obj) {
	// std::cout << "disown()" << std::endl;
	Unref();
	cache_.evict(obj);
}

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	repo_class_symbol = NODE_PSYMBOL("Repository");
	path_symbol = NODE_PSYMBOL("path");
	bare_symbol = NODE_PSYMBOL("bare");
	git_dir_symbol = NODE_PSYMBOL("gitDirectory");
	object_dir_symbol = NODE_PSYMBOL("objectDirectory");
	index_file_symbol = NODE_PSYMBOL("indexFile");
	work_dir_symbol = NODE_PSYMBOL("workDir");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(repo_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);
/*
	NODE_SET_PROTOTYPE_METHOD(t, "getTree", GetTree);
	NODE_SET_PROTOTYPE_METHOD(t, "getTag", GetTag);
	NODE_SET_PROTOTYPE_METHOD(t, "getReference", GetReference);
	NODE_SET_PROTOTYPE_METHOD(t, "getBlob", GetBlob);

	NODE_SET_PROTOTYPE_METHOD(t, "createWalker", CreateWalker);
	NODE_SET_PROTOTYPE_METHOD(t, "createTag", CreateTag);
	NODE_SET_PROTOTYPE_METHOD(t, "createTree", CreateTree);
	NODE_SET_PROTOTYPE_METHOD(t, "createBlob", CreateBlob);
	NODE_SET_PROTOTYPE_METHOD(t, "createCommit", CreateCommit);
	NODE_SET_PROTOTYPE_METHOD(t, "createOidReference", CreateOidRef);
	NODE_SET_PROTOTYPE_METHOD(t, "createSymbolicReference", CreateSymbolicRef);

	NODE_SET_PROTOTYPE_METHOD(t, "listReferences", ListReferences);
	NODE_SET_PROTOTYPE_METHOD(t, "packReferences", PackReferences);
	*/

	NODE_SET_PROTOTYPE_METHOD(t, "object", GetObject);
	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);

	// NODE_SET_PROTOTYPE_METHOD(t, "getIndex", GetIndex);

	NODE_SET_METHOD(target, "openRepository", OpenRepository);
	NODE_SET_METHOD(target, "initRepository", InitRepository);

	target->Set(repo_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_EXT_ARG(0, repoArg);
	Handle<Object> me = args.This();

	git_repository *repo = static_cast<git_repository*>(repoArg->Value());
	git_odb *odb;
	if(git_repository_odb(&odb, repo) != GIT_OK) {
		return scope.Close(ThrowGitError());
	}

	// Initialize our wrapped Repository class, which will then be wrapped in JS
	Repository *repoObj = new Repository();
	repoObj->Wrap(me);
	repoObj->repo_ = repo;
	repoObj->odb_ = odb;

	bool bare = git_repository_is_bare(repo);
	ImmutableSet(me, path_symbol, CastToJS(git_repository_path(repo)));
	ImmutableSet(me, bare_symbol, CastToJS((bool)bare));
	const char *workDir = git_repository_workdir(repo);
	if(workDir) ImmutableSet(me, work_dir_symbol, CastToJS(workDir));

	return args.This();
}

Handle<Value> Repository::OpenRepository(const Arguments& args) {
	HandleScope scope;

	string path = CastFromJS<string>(args[0]);
	OpenRepoBaton *baton = new OpenRepoBaton(path);
	baton->setCallback(args[1]);
	uv_queue_work(uv_default_loop(), &baton->req, AsyncOpenRepository,
		AsyncAfterOpenRepository);
	return Undefined();
}

void Repository::AsyncOpenRepository(uv_work_t *req) {
	OpenRepoBaton *baton = GetBaton<OpenRepoBaton>(req);

	AsyncLibCall(git_repository_open(&baton->repo, baton->path.c_str()), baton);
}

void Repository::AsyncAfterOpenRepository(uv_work_t *req) {
	HandleScope scope;
	OpenRepoBaton *baton = GetBaton<OpenRepoBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		// Call the Repository JS constructor to get our JS object.
		Handle<Value> constructorArgs[] = { External::New(baton->repo) };
		Local<Object> obj = Repository::constructor_template->GetFunction()
						->NewInstance(1, constructorArgs);

		Handle<Value> argv[] = { Null(), obj };
		FireCallback(baton->callback, 2, argv);
	}

	delete baton;
}

Handle<Value> Repository::InitRepository(const Arguments& args) {
	HandleScope scope;
	InitRepoBaton *baton = new InitRepoBaton;
	baton->path = CastFromJS<string>(args[0]);
	baton->bare = CastFromJS<bool>(args[1]);
	baton->setCallback(args[2]);
	uv_queue_work(uv_default_loop(), &baton->req, AsyncInitRepository,
		AsyncAfterInitRepository);
	return Undefined();
}

void Repository::AsyncInitRepository(uv_work_t *req) {
	InitRepoBaton *baton = GetBaton<InitRepoBaton>(req);

	AsyncLibCall(git_repository_init(&baton->repo, baton->path.c_str(), 
		baton->bare), baton);
}

void Repository::AsyncAfterInitRepository(uv_work_t *req) {
	HandleScope scope;
	InitRepoBaton *baton = GetBaton<InitRepoBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		Handle<Value> constructorArgs[] = { External::New(baton->repo) };
		Handle<Object> obj = Repository::constructor_template->GetFunction()
						->NewInstance(1, constructorArgs);

		Handle<Value> argv[] = { Null(), obj };
		FireCallback(baton->callback, 2, argv);
	}

	delete baton;
}

Handle<Value> Repository::GetObject(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	GetObjectBaton *baton = new GetObjectBaton(repo, CastFromJS<git_oid>(args[0]));
	baton->setCallback(args[1]);
	uv_queue_work(uv_default_loop(), &baton->req, AsyncGetObject, 
		AsyncAfterGetObject);
	return Undefined();
}

void Repository::AsyncGetObject(uv_work_t *req) {
	GetObjectBaton *baton = GetBaton<GetObjectBaton>(req);

	baton->repo->lockRepository();
	AsyncLibCall(git_object_lookup(&baton->object, baton->repo->repo_, 
		&baton->oid, GIT_OBJ_ANY), baton);
	baton->repo->unlockRepository();
}

void Repository::AsyncAfterGetObject(uv_work_t *req) {
	HandleScope scope;
	GetObjectBaton *baton = GetBaton<GetObjectBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		GitObject *obj;
		Handle<Value> ref = baton->repo->cache_.wrap(baton->object, &obj);
		if(obj == NULL) {
			return;
		}

		// For some reason, wrapping a Local around Node ObjectWrap's weakly 
		// held handle_ doesn't play too nicely with V8, I was getting some
		// really weird segfaults in V8 internals during the regular unit tests.
		// Temporarily allocating a Persistent ref during the lifetime of the 
		// callback seems to be working for now.
		Persistent<Value> strongRef = Persistent<Value>::New(ref);
		Handle<Value> argv[] = { Null(), Local<Value>::New(obj->handle_) };
		FireCallback(baton->callback, 2, argv);
		strongRef.Dispose();
		strongRef.Clear();
	}

	delete baton;
}

Handle<Value> Repository::Exists(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	ExistsBaton *baton = new ExistsBaton(repo, CastFromJS<git_oid>(args[0]));
	baton->setCallback(args[1]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncExists,
		AsyncAfterExists);
	return Undefined();
}

void Repository::AsyncExists(uv_work_t *req) {
	ExistsBaton *baton = GetBaton<ExistsBaton>(req);
	baton->exists = git_odb_exists(baton->repo->odb_, &baton->oid);
}

void Repository::AsyncAfterExists(uv_work_t *req) {
	HandleScope scope;
	ExistsBaton *baton = GetBaton<ExistsBaton>(req);

	Handle<Value> argv[] = { Null(), Boolean::New(baton->exists) };
	FireCallback(baton->callback, 2, argv);

	baton->callback.Dispose();
	delete baton;
}

void Repository::lockRepository() {
	LOCK_MUTEX(gitLock_);
}

void Repository::unlockRepository() {
	UNLOCK_MUTEX(gitLock_);
}

} // namespace gitteh
