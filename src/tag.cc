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

#define MESSAGE_PROPERTY String::NewSymbol("message")
#define NAME_PROPERTY String::NewSymbol("name")
#define ID_PROPERTY String::NewSymbol("id")
#define TAGGER_PROPERTY String::NewSymbol("tagger")
#define TARGET_PROPERTY String::NewSymbol("targetId")
#define TARGET_TYPE_PROPERTY String::NewSymbol("targetType")

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
	Tag *tag;
	git_oid targetId;
	std::string *name;
	git_signature *tagger;
	std::string *message;
	int error;
};

Persistent<FunctionTemplate> Tag::constructor_template;

void Tag::Init(Handle<Object>) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("Tag"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> Tag::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTag);

	Tag *tag = new Tag();
	tag->tag_ = static_cast<git_tag*>(theTag->Value());

	tag->Wrap(args.This());
	return args.This();
}

Handle<Value> Tag::Save(const Arguments& args) {
	HandleScope scope;

	Tag *tag = ObjectWrap::Unwrap<Tag>(args.This());

	CHECK_PROPERTY(NAME_PROPERTY);
	Handle<String> name = args.This()->Get(NAME_PROPERTY)->ToString();
	if(name->Length() == 0) {
		THROW_ERROR("Name must not be empty.");
	}
	GET_SIGNATURE_PROPERTY(TAGGER_PROPERTY, tagger);
	Handle<String> targetIdStr = args.This()->Get(TARGET_PROPERTY)->ToString();

	git_oid targetId;
	int res = git_oid_mkstr(&targetId, *String::Utf8Value(targetIdStr));
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Target id is invalid.", res);

	if(HAS_CALLBACK_ARG) {
		save_request *request = new save_request;
		REQ_FUN_ARG(args.Length() - 1, callbackArg);
		request->tag = tag;
		request->callback = Persistent<Function>::New(callbackArg);

		request->name = new std::string(*String::Utf8Value(name));
		memcpy(&request->targetId, &targetId, sizeof(git_oid));
		request->tagger = tagger;

		if((!args.This()->Get(MESSAGE_PROPERTY)->IsUndefined()) &&
				(!args.This()->Get(MESSAGE_PROPERTY)->IsNull())) {
			Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
			request->message = new std::string(*String::Utf8Value(message));
		}
		else {
			request->message = NULL;
		}

		tag->Ref();
		eio_custom(EIO_Save, EIO_PRI_DEFAULT, EIO_AfterSave, request);
		ev_ref(EV_DEFAULT_UC);

		return Undefined();
	}
	else {

		git_object *targetObj;
		res = git_object_lookup(&targetObj, tag->repository_->repo_, &targetId, GIT_OBJ_ANY);
		if(res != GIT_SUCCESS)
			THROW_GIT_ERROR("Couldn't get target object.", res);

		git_tag_set_target(tag->tag_, targetObj);
		git_tag_set_name(tag->tag_, *String::Utf8Value(name));
		git_tag_set_tagger(tag->tag_, tagger);

		if((!args.This()->Get(MESSAGE_PROPERTY)->IsUndefined()) &&
				(!args.This()->Get(MESSAGE_PROPERTY)->IsNull())) {
			Handle<String> message = args.This()->Get(MESSAGE_PROPERTY)->ToString();
			git_tag_set_message(tag->tag_, *String::Utf8Value(message));
		}

		res = git_object_write((git_object *)tag->tag_);
		if(res != GIT_SUCCESS) {
			THROW_GIT_ERROR("Couldn't save tag", res);
		}

		// Update tag id and type.
		args.This()->ForceSet(TARGET_TYPE_PROPERTY, String::New(git_object_type2string(git_tag_type(tag->tag_))), (PropertyAttribute)(ReadOnly | DontDelete));
		const git_oid *tagOid = git_tag_id(tag->tag_);
		char oidStr[40];
		git_oid_fmt(oidStr, tagOid);
		args.This()->ForceSet(ID_PROPERTY, String::New(oidStr, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

		return Undefined();
	}
}

int Tag::EIO_Save(eio_req *req) {
	save_request *reqData = static_cast<save_request*>(req->data);

	git_object *object;

	reqData->tag->repository_->lockRepository();
	reqData->error = git_object_lookup(&object, reqData->tag->repository_->repo_,
			&reqData->targetId, GIT_OBJ_ANY);
	if(reqData->error == GIT_SUCCESS) {
		git_tag_set_target(reqData->tag->tag_, object);
		git_tag_set_name(reqData->tag->tag_, reqData->name->c_str());
		git_tag_set_tagger(reqData->tag->tag_, reqData->tagger);

		if(reqData->message) {
			git_tag_set_message(reqData->tag->tag_, reqData->message->c_str());
			delete reqData->message;
		}

		reqData->error = git_object_write((git_object*)reqData->tag->tag_);
	}

	reqData->tag->repository_->unlockRepository();

	delete reqData->name;
	git_signature_free(reqData->tagger);

	return 0;
}

int Tag::EIO_AfterSave(eio_req *req) {
	HandleScope scope;
	save_request *reqData = static_cast<save_request*>(req->data);

	ev_unref(EV_DEFAULT_UC);
 	reqData->tag->Unref();

	Handle<Value> callbackArgs[2];
 	if(reqData->error != GIT_SUCCESS) {
 		Handle<Value> error = CreateGitError(String::New("Couldn't save tag."), reqData->error);
 		callbackArgs[0] = error;
 		callbackArgs[1] = Null();
	}
	else {
		reqData->tag->repository_->lockRepository();

		// Update tag id and type.
		reqData->tag->handle_->ForceSet(TARGET_TYPE_PROPERTY,
				String::New(git_object_type2string(git_tag_type(reqData->tag->tag_))),
				(PropertyAttribute)(ReadOnly | DontDelete));
		const git_oid *tagOid = git_tag_id(reqData->tag->tag_);
		char oidStr[40];
		git_oid_fmt(oidStr, tagOid);
		reqData->tag->handle_->ForceSet(ID_PROPERTY, String::New(oidStr, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

		reqData->tag->repository_->unlockRepository();

 		callbackArgs[0] = Null();
 		callbackArgs[1] = True();
	}

	reqData->callback.Dispose();
	TRIGGER_CALLBACK();
	delete reqData;

	return 0;
}

void Tag::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	if(data != NULL) {
		tag_data *tagData = static_cast<tag_data*>(data);
		jsObject->Set(ID_PROPERTY, String::New(tagData->id, 40), (PropertyAttribute)(ReadOnly | DontDelete));

		jsObject->Set(NAME_PROPERTY, String::New(tagData->name->c_str()));
		jsObject->Set(MESSAGE_PROPERTY, String::New(tagData->message->c_str()));

		CREATE_PERSON_OBJ(taggerObj, tagData->tagger);
		jsObject->Set(TAGGER_PROPERTY, taggerObj);

		jsObject->Set(TARGET_PROPERTY, String::New(tagData->targetId, 40));
		jsObject->Set(TARGET_TYPE_PROPERTY, String::New(tagData->targetType->c_str()), (PropertyAttribute)(ReadOnly | DontDelete));

		delete tagData->targetType;
		delete tagData->name;
		delete tagData->message;
		git_signature_free(tagData->tagger);
		delete tagData;
	}
	else {
		jsObject->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObject->Set(NAME_PROPERTY, Null());
		jsObject->Set(MESSAGE_PROPERTY, Null());
		jsObject->Set(TAGGER_PROPERTY, Null());
		jsObject->Set(TARGET_PROPERTY, Null());
		jsObject->Set(TARGET_TYPE_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
	}
}

void* Tag::loadInitData() {
	tag_data *data = new tag_data;

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

	return data;
}

void Tag::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
