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

#include "tag.h"
#include "repository.h"
#include "signature.h"

static Persistent<String> message_symbol;
static Persistent<String> name_symbol;
static Persistent<String> id_symbol;
static Persistent<String> tagger_symbol;
static Persistent<String> targetId_symbol;
static Persistent<String> targetType_symbol;

namespace gitteh {

struct tag_data {
	char id[40];
	std::string *name;
	std::string *message;
	char targetId[40];
	std::string *targetType;
	git_signature *tagger;
};

struct save_request {
	Persistent<Function> callback;
	Repository *repo;
	Persistent<Object> repoHandle;
	Tag *tag;
	git_oid targetId;
	git_otype targetType;
	std::string *name;
	git_signature *tagger;
	std::string *message;
	int error;
	bool isNew;
	char id[40];
};

Persistent<FunctionTemplate> Tag::constructor_template;

Tag::Tag(git_tag *tag) {
	tag_ = tag;
}

Tag::~Tag() {
	repository_->lockRepository();
	git_tag_close(tag_);
	repository_->unlockRepository();
}

void Tag::Init(Handle<Object>) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("Tag"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);

	message_symbol = NODE_PSYMBOL("message");
	name_symbol = NODE_PSYMBOL("name");
	id_symbol = NODE_PSYMBOL("id");
	tagger_symbol = NODE_PSYMBOL("tagger");
	targetId_symbol = NODE_PSYMBOL("targetId");
	targetType_symbol = NODE_PSYMBOL("targetType");
}

Handle<Value> Tag::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, tagArg);

	Tag *tag = static_cast<Tag*>(tagArg->Value());
	tag->Wrap(args.This());

	tag->processInitData();

	return args.This();
}

Handle<Value> Tag::SaveObject(Handle<Object> tagObject, Repository *repo,
		Handle<Value> callback, bool isNew) {
	HandleScope scope;
	int result;

	if(!tagObject->Has(name_symbol)) {
		THROW_ERROR("Tag requires a name.");
	}

	if(!tagObject->Has(targetId_symbol)) {
		THROW_ERROR("Tag requires a target.");
	}

	if(!tagObject->Has(tagger_symbol)) {
		THROW_ERROR("Tag requires a tagger.");
	}

	if(!tagObject->Has(message_symbol)) {
		THROW_ERROR("Tag requires a message.");
	}

	String::Utf8Value name(tagObject->Get(name_symbol));
	if(!name.length()) {
		THROW_ERROR("Tag requires a name.");
	}

	String::Utf8Value targetId(tagObject->Get(targetId_symbol));
	if(!targetId.length()) {
		THROW_ERROR("Tag requires a target.");
	}

	String::Utf8Value message(tagObject->Get(message_symbol));
	if(!message.length()) {
		THROW_ERROR("Tag requires a message.");
	}

	git_object* targetObj;
	git_oid targetOid;
	git_oid_mkstr(&targetOid, *targetId);
	result = git_object_lookup(&targetObj, repo->repo_, &targetOid, GIT_OBJ_ANY);
	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't find target object.", result);
	}

	git_otype targetType = git_object_type(targetObj);
	git_object_close(targetObj);

	git_signature *tagger = GetSignatureFromProperty(tagObject, tagger_symbol);
	if(tagger == NULL) {
		THROW_ERROR("Tagger is not a valid signature.");
	}

	if(callback->IsFunction()) {
		save_request *request = new save_request;
		request->repo = repo;
		request->repoHandle = Persistent<Object>::New(repo->handle_);


		request->message = new std::string(*message);
		request->name = new std::string(*name);
		request->tagger = tagger;
		request->targetId = targetOid;
		request->targetType = targetType;

		request->isNew = isNew;
		if(!isNew) {
			request->tag = ObjectWrap::Unwrap<Tag>(tagObject);
			request->tag->Ref();
		}
		else {
			request->tag = NULL;
		}

		request->callback = Persistent<Function>::New(Handle<Function>::Cast(callback));
		request->repo = repo;
		request->repoHandle = Persistent<Object>::New(repo->handle_);

		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {
		repo->lockRepository();

		git_oid newId;
		result = git_tag_create(&newId, repo->repo_, *name, &targetOid,
				targetType, tagger, *message);
		repo->unlockRepository();

		git_signature_free(tagger);

		if(result != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't save tag.", result);
		}

		char newIdStr[40];
		git_oid_fmt(newIdStr, &newId);
		if(isNew) {
			Handle<Function> getTagFn = Handle<Function>::Cast(
					repo->handle_->Get(String::New("getTag")));
			Handle<Value> arg = String::New(newIdStr, 40);
			return scope.Close(getTagFn->Call(repo->handle_, 1,
					&arg));
		}
		else {
			tagObject->ForceSet(id_symbol, String::New(newIdStr, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));

			tagObject->ForceSet(targetType_symbol, String::New(
					git_object_type2string(targetType)),
					(PropertyAttribute)(ReadOnly | DontDelete));

			return scope.Close(True());
		}
	}
}

Handle<Value> Tag::Save(const Arguments& args) {
	HandleScope scope;
	Tag *tag = ObjectWrap::Unwrap<Tag>(args.This());

	Handle<Value> callback = Null();
	if(HAS_CALLBACK_ARG) {
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		callback = callbackArg;
	}

	return scope.Close(SaveObject(args.This(), tag->repository_, callback, false));
}

int Tag::EIO_Save(eio_req *req) {
	save_request *reqData = static_cast<save_request*>(req->data);

	git_oid newId;
	reqData->repo->lockRepository();
	reqData->error = git_tag_create(&newId, reqData->repo->repo_, reqData->name->c_str(),
			&reqData->targetId, reqData->targetType, reqData->tagger,
			reqData->message->c_str());
	reqData->repo->unlockRepository();

	if(reqData->error == GIT_SUCCESS) {
		git_oid_fmt(reqData->id, &newId);
	}

	delete reqData->name;
	delete reqData->message;
	git_signature_free(reqData->tagger);

	return 0;
}

int Tag::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_request *reqData = static_cast<save_request*>(req->data);

	reqData->repoHandle.Dispose();
	ev_unref(EV_DEFAULT_UC);
	if(reqData->tag != NULL) reqData->tag->Unref();

	Handle<Value> callbackArgs[2];
	if(reqData->error != GIT_SUCCESS) {
		Handle<Value> error = Exception::Error(String::New("Couldn't save tag."));
		callbackArgs[0] = error;
		callbackArgs[1] = Null();
	}
	else {
		callbackArgs[0] = Null();

		if(reqData->isNew) {
			Handle<Function> getTagFn = Handle<Function>::Cast(
					reqData->repo->handle_->Get(String::New("getTag")));
			Handle<Value> arg = String::New(reqData->id, 40);
			callbackArgs[1] = getTagFn->Call(reqData->repo->handle_, 1, &arg);
		}
		else {
			reqData->tag->handle_->ForceSet(id_symbol, String::New(reqData->id, 40),
					(PropertyAttribute)(ReadOnly | DontDelete));
			reqData->tag->handle_->ForceSet(targetType_symbol, String::New(
					git_object_type2string(reqData->targetType)),
					(PropertyAttribute)(ReadOnly | DontDelete));

			callbackArgs[1] = True();
		}
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

void Tag::processInitData() {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	tag_data *tagData = initData_;
	jsObject->Set(id_symbol, String::New(tagData->id, 40), (PropertyAttribute)(ReadOnly | DontDelete));

	jsObject->Set(name_symbol, String::New(tagData->name->c_str()));
	jsObject->Set(message_symbol, String::New(tagData->message->c_str()));

	CREATE_PERSON_OBJ(taggerObj, tagData->tagger);
	jsObject->Set(tagger_symbol, taggerObj);

	jsObject->Set(targetId_symbol, String::New(tagData->targetId, 40));
	jsObject->Set(targetType_symbol, String::New(tagData->targetType->c_str()), (PropertyAttribute)(ReadOnly | DontDelete));

	delete tagData->targetType;
	delete tagData->name;
	delete tagData->message;
	git_signature_free(tagData->tagger);
	delete tagData;
}

int Tag::doInit() {
	tag_data *data = initData_ = new tag_data;

	repository_->lockRepository();
	const git_oid *tagOid = git_tag_id(tag_);
	git_object *object;
	git_tag_target(&object, tag_);
	const git_oid *targetId = git_object_id(const_cast<git_object*>(object));

	git_oid_fmt(data->id, tagOid);
	git_oid_fmt(data->targetId, targetId);

	data->name = new std::string(git_tag_name(tag_));
	data->message = new std::string(git_tag_message(tag_));
	data->targetType = new std::string(git_object_type2string(git_tag_type(tag_)));

	const git_signature *tagger = git_tag_tagger(tag_);
	data->tagger = git_signature_dup(tagger);

	repository_->unlockRepository();

	return GIT_SUCCESS;
}

void Tag::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
