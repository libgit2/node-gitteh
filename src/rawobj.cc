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

#include "rawobj.h"
#include "repository.h"
#include <node_buffer.h>

#define ID_PROPERTY String::NewSymbol("id")
#define DATA_PROPERTY String::NewSymbol("data")
#define TYPE_PROPERTY String::NewSymbol("type")

namespace gitteh {

Persistent<FunctionTemplate> RawObject::constructor_template;

void RawObject::Init(Handle<Object> target) {
	HandleScope scope;

	Local<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("RawObject"));
	t->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "save", Save);
}

Handle<Value> RawObject::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theObj);

	RawObject *obj = new RawObject();
	obj->Wrap(args.This());

	obj->obj_ = static_cast<git_rawobj*>(theObj->Value());

	return args.This();
}

Handle<Value> RawObject::Save(const Arguments& args) {
	HandleScope scope;

	RawObject *obj = ObjectWrap::Unwrap<RawObject>(args.This());

	Handle<Object> thisObj = args.This();

	CHECK_PROPERTY(DATA_PROPERTY);
	CHECK_PROPERTY(TYPE_PROPERTY);

	if(!Buffer::HasInstance(thisObj->Get(DATA_PROPERTY))) {
		THROW_ERROR("Data is not a valid Buffer.");
	}

	Handle<Object> bufferObj = Handle<Object>::Cast(
			thisObj->Get(DATA_PROPERTY));

	git_otype type = git_object_string2type(*String::Utf8Value(
			thisObj->Get(TYPE_PROPERTY)));

	if(type == GIT_OBJ_BAD) {
		THROW_ERROR("Invalid type.");
	}

	obj->obj_->type = type;
	obj->obj_->len = Buffer::Length(bufferObj);
	obj->obj_->data = Buffer::Data(bufferObj);

	git_oid newId;
	int res = git_odb_write(&newId, obj->repository_->odb_, obj->obj_);
	if(res != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't save raw object.", res);

	const char* oidStr = git_oid_allocfmt(&newId);
	args.This()->ForceSet(ID_PROPERTY, String::New(oidStr), (PropertyAttribute)(ReadOnly | DontDelete));

	return Undefined();
}

RawObject::~RawObject() {
	delete obj_;
}

struct rawobj_data {
	char id[40];
	std::string *type;
	int len;
	void *data;
};

void RawObject::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	if(data != NULL) {
		rawobj_data *rawObjData = static_cast<rawobj_data*>(data);

		jsObject->Set(ID_PROPERTY, String::New(rawObjData->id, 40),
				(PropertyAttribute)(ReadOnly | DontDelete));

		Buffer *buf = Buffer::New(static_cast<char*>(rawObjData->data),
				rawObjData->len);
		Local<Object> globalObj = Context::GetCurrent()->Global();
		Local<Function> bufferConstructor = Local<Function>::Cast(
				globalObj->Get(String::New("Buffer")));
		Handle<Value> constructorArgs[3] = { buf->handle_, Integer::New(
				rawObjData->len), Integer::New(0) };
		Local<Object> actualBuffer = bufferConstructor->NewInstance(3, constructorArgs);
		jsObject->Set(DATA_PROPERTY, actualBuffer);
		jsObject->Set(TYPE_PROPERTY, String::New(rawObjData->type->c_str()));

		delete rawObjData->type;
		delete rawObjData;
	}
	else {
		jsObject->Set(ID_PROPERTY, Null(), (PropertyAttribute)(ReadOnly | DontDelete));
		jsObject->Set(DATA_PROPERTY, Null());

		repository_->lockRepository();
		jsObject->Set(TYPE_PROPERTY, String::New(git_object_type2string(GIT_OBJ_BAD)));
		repository_->unlockRepository();
	}
}

void* RawObject::loadInitData() {
	rawobj_data *data = new rawobj_data;

	repository_->lockRepository();
	git_oid id;
	int res = git_rawobj_hash(&id, obj_);
	if(res == GIT_SUCCESS) {
		// TODO: what should we do if this fails?
		git_oid_fmt(data->id, &id);
	}

	data->type = new std::string(git_object_type2string(obj_->type));
	data->len = obj_->len;
	data->data = malloc(data->len);
	memcpy(data->data, obj_->data, data->len);

	repository_->unlockRepository();

	return data;
}

void RawObject::setOwner(void *owner) {
	repository_ = static_cast<Repository*>(owner);
}

} // namespace gitteh
