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
#include "commit.h"
#include "tree.h"
#include "blob.h"
#include "tag.h"
#include "remote.h"
#include "index.h"

using std::list;

namespace gitteh {
static Persistent<String> repo_class_symbol;
static Persistent<String> path_symbol;
static Persistent<String> bare_symbol;
static Persistent<String> index_symbol;

static Persistent<String> object_dir_symbol;
static Persistent<String> index_file_symbol;
static Persistent<String> work_dir_symbol;
static Persistent<String> remotes_symbol;
static Persistent<String> references_symbol;

static Persistent<String> ref_name_symbol;
static Persistent<String> ref_direct_symbol;
static Persistent<String> ref_packed_symbol;
static Persistent<String> ref_target_symbol;

static Persistent<String> object_id_symbol;
static Persistent<String> object_type_symbol;

class OpenRepoBaton : public Baton {
public:
	string path	;
	git_repository *repo;
	list<string> remotes;
	list<string> references;

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
	char oidLength;
	git_object *object;
	git_otype type;
	GetObjectBaton(Repository *r, git_oid oid) : RepositoryBaton(r), oid(oid) {}
};

class ReferenceBaton : public RepositoryBaton {
public:
	git_reference *ref;
	ReferenceBaton(Repository *r) : RepositoryBaton(r) {
		ref = NULL;
	}
	~ReferenceBaton() {
		if(ref) {
			git_reference_free(ref);
		}
	}
};

class GetReferenceBaton : public ReferenceBaton {
public:
	string name;
	bool resolve;

	GetReferenceBaton(Repository *r, string _name) : ReferenceBaton(r), name(_name) {}
};

class CreateReferenceBaton : public ReferenceBaton {
public:
	bool direct_;
	string name_;
	git_oid targetId_;
	string target_;
	bool force_;

	CreateReferenceBaton(Repository *r, string name, git_oid target,
			bool force) : ReferenceBaton(r) {
		name_ = name;
		targetId_ = target;
		force_ = force;
		direct_ = true;
	}

	CreateReferenceBaton(Repository *r, string name, string target,
			bool force) : ReferenceBaton(r) {
		name_ = name;
		target_ = target;
		force_ = force;
		direct_ = false;
	}
};

class CreateSymReferenceBaton : public ReferenceBaton {
public:
	string name;
	string target;

	CreateSymReferenceBaton(Repository *r, string name, string target) :
			ReferenceBaton(r), name(name), target(target) { }
};

class GetRemoteBaton : public RepositoryBaton {
public:
	string name;
	git_remote *remote;

	GetRemoteBaton(Repository *r, string _name) : RepositoryBaton(r), name(_name) {}
};

class CreateRemoteBaton : public RepositoryBaton {
public:
	string name;
	string url;
	git_remote *remote;
	CreateRemoteBaton(Repository *r) : RepositoryBaton(r) { }
};

Persistent<FunctionTemplate> Repository::constructor_template;

Repository::Repository() {
	CREATE_MUTEX(gitLock_);

	odb_ = NULL;
	repo_ = NULL;
}

Repository::~Repository() {
	if(odb_) {
		git_odb_free(odb_);
		odb_ = NULL;
	}

	if(index_) {
		git_index_free(index_);
		index_ = NULL;
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

void Repository::Init(Handle<Object> target) {
	HandleScope scope;

	repo_class_symbol 	= NODE_PSYMBOL("NativeRepository");
	path_symbol 		= NODE_PSYMBOL("path");
	bare_symbol 		= NODE_PSYMBOL("bare");
	object_dir_symbol 	= NODE_PSYMBOL("objectDirectory");
	index_file_symbol 	= NODE_PSYMBOL("indexFile");
	work_dir_symbol 	= NODE_PSYMBOL("workDir");
	remotes_symbol 		= NODE_PSYMBOL("remotes");
	index_symbol		= NODE_PSYMBOL("index");
	references_symbol	= NODE_PSYMBOL("references");

	// Reference symbols
	ref_name_symbol 	= NODE_PSYMBOL("name");
	ref_direct_symbol 	= NODE_PSYMBOL("direct");
	ref_packed_symbol 	= NODE_PSYMBOL("packed");
	ref_target_symbol 	= NODE_PSYMBOL("target");

	// Object symbols
	object_id_symbol	= NODE_PSYMBOL("id");
	object_type_symbol	= NODE_PSYMBOL("_type");

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(repo_class_symbol);
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "object", GetObject);
	NODE_SET_PROTOTYPE_METHOD(t, "exists", Exists);
	NODE_SET_PROTOTYPE_METHOD(t, "reference", GetReference);
	NODE_SET_PROTOTYPE_METHOD(t, "createOidReference", CreateOidReference);
	NODE_SET_PROTOTYPE_METHOD(t, "createSymReference", CreateSymReference);
	NODE_SET_PROTOTYPE_METHOD(t, "remote", GetRemote);
	NODE_SET_PROTOTYPE_METHOD(t, "createRemote", CreateRemote);

	NODE_SET_METHOD(target, "openRepository", OpenRepository);
	NODE_SET_METHOD(target, "initRepository", InitRepository);

	target->Set(repo_class_symbol, constructor_template->GetFunction());
}

Handle<Value> Repository::New(const Arguments& args) {
	HandleScope scope;

	REQ_EXT_ARG(0, repoArg);
	REQ_EXT_ARG(1, refsArg);
	REQ_EXT_ARG(2, remotesArg);
	Handle<Object> me = args.This();

	git_repository *repo = static_cast<git_repository*>(repoArg->Value());
	git_odb *odb;
	git_index *index;
	if(git_repository_odb(&odb, repo) != GIT_OK) {
		return scope.Close(ThrowGitError());
	}
	if(git_repository_index(&index, repo) != GIT_OK) {
		return scope.Close(ThrowGitError());
	}

	// Initialize our wrapped Repository class, which will then be wrapped in JS
	Repository *repoObj = new Repository();
	repoObj->Wrap(me);
	repoObj->repo_ = repo;
	repoObj->odb_ = odb;
	repoObj->index_ = index;

	bool bare = git_repository_is_bare(repo);
	ImmutableSet(me, path_symbol, CastToJS(git_repository_path(repo)));
	ImmutableSet(me, bare_symbol, CastToJS((bool)bare));
	const char *workDir = git_repository_workdir(repo);
	if(workDir) ImmutableSet(me, work_dir_symbol, CastToJS(workDir));

	list<string> *remotes = static_cast<list<string>*>(remotesArg->Value());
	if(remotes != NULL) {
		ImmutableSet(me, remotes_symbol, CastToJS(*remotes));
	}
	else {
		ImmutableSet(me, remotes_symbol, Array::New());
	}

	list<string> *references = static_cast<list<string>*>(refsArg->Value());
	if(references != NULL) {
		me->Set(references_symbol, CastToJS(*references));
	}
	else {
		me->Set(references_symbol, Array::New());
	}

	Handle<Value> constructorArgs[] = {
		External::New(repoObj),
		External::New(index)
	};
	Local<Object> indexObj = Index::constructor_template->GetFunction()
			->NewInstance(2, constructorArgs);

	me->Set(index_symbol, indexObj);

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

	if(AsyncLibCall(git_repository_open(&baton->repo, baton->path.c_str()),
			baton)) {
		git_strarray strarray;
		if(AsyncLibCall(git_remote_list(&strarray, baton->repo), baton)) {
			for(unsigned int i = 0; i < strarray.count; i++) {
				baton->remotes.push_back(string(strarray.strings[i]));
			}
			git_strarray_free(&strarray);
		}

		if(AsyncLibCall(git_reference_list(&strarray, baton->repo,
				GIT_REF_LISTALL), baton)) {
			for(unsigned int i = 0; i < strarray.count; i++) {
				baton->references.push_back(string(strarray.strings[i]));
			}
			git_strarray_free(&strarray);
		}
	}
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
		Handle<Value> constructorArgs[] = {
			External::New(baton->repo),
			External::New(&baton->references),
			External::New(&baton->remotes)
		};
		Local<Object> obj = Repository::constructor_template->GetFunction()
						->NewInstance(3, constructorArgs);

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
		Handle<Value> constructorArgs[] = {
			External::New(baton->repo),
			External::New(NULL),
			External::New(NULL)
		};
		Handle<Object> obj = Repository::constructor_template->GetFunction()
						->NewInstance(3, constructorArgs);

		Handle<Value> argv[] = { Null(), obj };
		FireCallback(baton->callback, 2, argv);
	}

	delete baton;
}

Handle<Value> Repository::GetObject(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	Handle<String> oidArg = Handle<String>::Cast(args[0]);
	GetObjectBaton *baton = new GetObjectBaton(repo, CastFromJS<git_oid>(args[0]));
	baton->type = CastFromJS<git_otype>(args[1]);
	baton->oidLength = oidArg->Length();
	baton->setCallback(args[2]);
	uv_queue_work(uv_default_loop(), &baton->req, AsyncGetObject, 
		AsyncAfterGetObject);
	return Undefined();
}

void Repository::AsyncGetObject(uv_work_t *req) {
	GetObjectBaton *baton = GetBaton<GetObjectBaton>(req);

	baton->repo->lockRepository();
	AsyncLibCall(git_object_lookup_prefix(&baton->object, baton->repo->repo_, 
		&baton->oid, baton->oidLength, baton->type), baton);
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
		git_object *gitObj = baton->object;
		Handle<Object> jsObj;
		git_otype type = git_object_type(gitObj);
		switch(type) {
			case GIT_OBJ_COMMIT: {
				jsObj = Commit::Create((git_commit*)gitObj);
				break;
			}
			case GIT_OBJ_TREE: {
				jsObj = Tree::Create((git_tree*)gitObj);
				break;
			}
			case GIT_OBJ_BLOB: {
				jsObj = Blob::Create((git_blob*)gitObj);
				break;
			}
			case GIT_OBJ_TAG: {
				jsObj = Tag::Create((git_tag*)gitObj);
				break;
			}
			default: {
				Handle<String> err = String::New("Invalid object.");
				Handle<Value> argv[] = { Exception::Error(err) };
				FireCallback(baton->callback, 1, argv);
				git_object_free(gitObj);
				return;
			}
		}

		jsObj->Set(object_id_symbol, CastToJS(git_object_id(gitObj)));
		jsObj->Set(object_type_symbol, Integer::New(git_object_type(gitObj)));

		git_object_free(gitObj);

		Handle<Value> argv[] = { Null(), Local<Value>::New(jsObj) };
		FireCallback(baton->callback, 2, argv);
	}

	delete baton;
}

Handle<Value> Repository::GetReference(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	GetReferenceBaton *baton = new GetReferenceBaton(repo, 
		CastFromJS<string>(args[0]));
	baton->resolve = CastFromJS<bool>(args[1]);
	baton->setCallback(args[2]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncGetReference,
		AsyncReturnReference);
	return Undefined();
}

void Repository::AsyncGetReference(uv_work_t *req) {
	GetReferenceBaton *baton = GetBaton<GetReferenceBaton>(req);

	git_reference *ref;
	if(AsyncLibCall(git_reference_lookup(&ref, baton->repo->repo_,
			baton->name.c_str()), baton)) {
		if(baton->resolve) {
			AsyncLibCall(git_reference_resolve(&baton->ref, ref), baton);
			git_reference_free(ref);
		}
		else {
			baton->ref = ref;
		}
	}
}

Handle<Value> Repository::CreateOidReference(const Arguments &args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	CreateReferenceBaton *baton = new CreateReferenceBaton(repo,
			CastFromJS<string>(args[0]), CastFromJS<git_oid>(args[1]),
			CastFromJS<bool>(args[2]));
	baton->setCallback(args[3]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncCreateReference,
			AsyncReturnReference);

	return Undefined();
}

Handle<Value> Repository::CreateSymReference(const Arguments &args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());
	CreateReferenceBaton *baton = new CreateReferenceBaton(repo,
			CastFromJS<string>(args[0]), CastFromJS<string>(args[1]),
			CastFromJS<bool>(args[2]));
	baton->setCallback(args[3]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncCreateReference,
			AsyncReturnReference);

	return Undefined();
}

void Repository::AsyncCreateReference(uv_work_t *req) {
	CreateReferenceBaton *baton = GetBaton<CreateReferenceBaton>(req);

	if(baton->direct_) {
		AsyncLibCall(git_reference_create_oid(&baton->ref, baton->repo->repo_,
				baton->name_.c_str(), &baton->targetId_, baton->force_), baton);
	}
	else {
		AsyncLibCall(git_reference_create_symbolic(&baton->ref,
				baton->repo->repo_, baton->name_.c_str(),
				baton->target_.c_str(), baton->force_), baton);
	}
}

void Repository::AsyncReturnReference(uv_work_t *req) {
	HandleScope scope;
	ReferenceBaton *baton = GetBaton<ReferenceBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		Handle<Value> argv[] = { Null(), CreateReferenceObject(baton->ref) };
		FireCallback(baton->callback, 2, argv);
	}

	// Deletion of this baton handles freeing the git_reference.
	delete baton;
}

Handle<Object> Repository::CreateReferenceObject(git_reference *ref) {
	HandleScope scope;

	Handle<Object> obj = Object::New();

	git_ref_t refType = git_reference_type(ref);
	obj->Set(ref_name_symbol, CastToJS(git_reference_name(ref)));
	obj->Set(ref_direct_symbol, CastToJS<bool>(refType == GIT_REF_OID));
	obj->Set(ref_packed_symbol, CastToJS<bool>(git_reference_is_packed(ref)));

	if(refType == GIT_REF_OID) {
		obj->Set(ref_target_symbol, CastToJS(git_reference_oid(ref)));
	}
	else {
		obj->Set(ref_target_symbol, CastToJS(git_reference_target(ref)));
	}

	return scope.Close(obj);
}

Handle<Value> Repository::GetRemote(const Arguments& args) {
	HandleScope scope;
	Repository *repo = ObjectWrap::Unwrap<Repository>(args.This());

	GetRemoteBaton *baton = new GetRemoteBaton(repo, 
		CastFromJS<string>(args[0]));
	baton->setCallback(args[1]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncGetRemote,
		AsyncAfterGetRemote);
	return Undefined();
}

void Repository::AsyncGetRemote(uv_work_t *req) {
	GetRemoteBaton *baton = GetBaton<GetRemoteBaton>(req);

	AsyncLibCall(git_remote_load(&baton->remote, baton->repo->repo_, 
		baton->name.c_str()), baton);
}

void Repository::AsyncAfterGetRemote(uv_work_t *req) {
	HandleScope scope;
	GetRemoteBaton *baton = GetBaton<GetRemoteBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		Handle<Value> constructorArgs[] = { External::New(baton->remote) };
		Local<Object> obj = Remote::constructor_template->GetFunction()
						->NewInstance(1, constructorArgs);

		Handle<Value> argv[] = { Null(), obj };
		FireCallback(baton->callback, 2, argv);
	}

	delete baton;
}

Handle<Value> Repository::CreateRemote(const Arguments &args) {
	HandleScope scope;
	Repository *repository = ObjectWrap::Unwrap<Repository>(args.This());

	CreateRemoteBaton *baton = new CreateRemoteBaton(repository);
	baton->name = CastFromJS<string>(args[0]);
	baton->url = CastFromJS<string>(args[1]);
	baton->setCallback(args[2]);

	uv_queue_work(uv_default_loop(), &baton->req, AsyncCreateRemote,
			AsyncAfterCreateRemote);

	return Undefined();
}

void Repository::AsyncCreateRemote(uv_work_t *req) {
	CreateRemoteBaton *baton = GetBaton<CreateRemoteBaton>(req);

	if(AsyncLibCall(git_remote_add(&baton->remote, baton->repo->repo_,
			baton->name.c_str(), baton->url.c_str()), baton)) {
		if(!AsyncLibCall(git_remote_save(baton->remote), baton)) {
			git_remote_free(baton->remote);
		}
	}
}

void Repository::AsyncAfterCreateRemote(uv_work_t *req) {
	HandleScope scope;
	CreateRemoteBaton *baton = GetBaton<CreateRemoteBaton>(req);

	if(baton->isErrored()) {
		Handle<Value> argv[] = { baton->createV8Error() };
		FireCallback(baton->callback, 1, argv);
	}
	else {
		Handle<Value> constructorArgs[] = { External::New(baton->remote) };
		Local<Object> obj = Remote::constructor_template->GetFunction()
						->NewInstance(1, constructorArgs);

		Handle<Value> argv[] = { Null(), obj };
		FireCallback(baton->callback, 2, argv);
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

	delete baton;
}

void Repository::lockRepository() {
	LOCK_MUTEX(gitLock_);
}

void Repository::unlockRepository() {
	UNLOCK_MUTEX(gitLock_);
}

} // namespace gitteh
