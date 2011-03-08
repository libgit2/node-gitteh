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

	const git_oid *tagOid = git_tag_id(tag->tag_);
	if(tagOid != NULL) {
		char *oidStr = git_oid_allocfmt(tagOid);
		args.This()->Set(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

		args.This()->Set(NAME_PROPERTY, String::New(git_tag_name(tag->tag_)));
		args.This()->Set(MESSAGE_PROPERTY, String::New(git_tag_message(tag->tag_)));

		git_signature *tagger = const_cast<git_signature*>(git_tag_tagger(tag->tag_));
		CREATE_PERSON_OBJ(taggerObj, tagger);
		args.This()->Set(TAGGER_PROPERTY, taggerObj);

		char *targetOidStr = git_oid_allocfmt(git_object_id(const_cast<git_object*>(git_tag_target(tag->tag_))));
		args.This()->Set(TARGET_PROPERTY, String::New(targetOidStr));
		args.This()->Set(TARGET_TYPE_PROPERTY, String::New(git_object_type2string(git_tag_type(tag->tag_))), (PropertyAttribute)(ReadOnly | DontDelete));
	}
	else {
		args.This()->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		args.This()->Set(NAME_PROPERTY, Null());
		args.This()->Set(MESSAGE_PROPERTY, Null());
		args.This()->Set(TAGGER_PROPERTY, Null());
		args.This()->Set(TARGET_PROPERTY, Null());
		args.This()->Set(TARGET_TYPE_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
	}

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

	git_object *targetObj;
	res = git_repository_lookup(&targetObj, tag->repository_->repo_, &targetId, GIT_OBJ_ANY);
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
	char *oidStr = git_oid_allocfmt(tagOid);
	args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));
}

} // namespace gitteh
