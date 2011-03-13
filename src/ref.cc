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

#include "ref.h"
#include "repository.h"

#define NAME_PROPERTY String::NewSymbol("name")
#define TYPE_PROPERTY String::NewSymbol("type")
#define TARGET_PROPERTY String::NewSymbol("target")

namespace gitteh {

struct ref_data {
	std::string *name;
	git_rtype type;
	std::string *target;
};

Persistent<FunctionTemplate> Reference::constructor_template;

Reference::Reference() {
	deleted_ = false;
}

void Reference::Init(Handle<Object> target) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Reference"));
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "rename", Rename);
	NODE_SET_PROTOTYPE_METHOD(t, "delete", Delete);
	NODE_SET_PROTOTYPE_METHOD(t, "resolve", Resolve);
	NODE_SET_PROTOTYPE_METHOD(t, "setTarget", SetTarget);

	NODE_DEFINE_CONSTANT(target, GIT_REF_OID);
	NODE_DEFINE_CONSTANT(target, GIT_REF_SYMBOLIC);
}

Handle<Value> Reference::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, refArg);

	Reference *ref = new Reference();
	ref->Wrap(args.This());
	ref->ref_ = static_cast<git_reference*>(refArg->Value());

	return args.This();
}

Handle<Value> Reference::Rename(const Arguments& args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());
	if(ref->deleted_) {
		THROW_ERROR("This ref has been deleted!");
	}
	
	REQ_ARGS(1);
	REQ_STR_ARG(0, newNameArg);

	int result = git_reference_rename(ref->ref_, *newNameArg);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't rename ref.", result);

	args.This()->ForceSet(NAME_PROPERTY, String::New(git_reference_name(ref->ref_)),
			(PropertyAttribute)(ReadOnly | DontDelete));
			
	return scope.Close(Undefined());
}

Handle<Value> Reference::Delete(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());
	if(ref->deleted_) {
		THROW_ERROR("This ref has already been deleted!");
	}

	int result = git_reference_delete(ref->ref_);
	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't delete ref.", result);
	}

	ref->deleted_ = true;
	return scope.Close(True());
}

Handle<Value> Reference::Resolve(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());
	if(ref->deleted_) {
		THROW_ERROR("This ref has been deleted!");
	}

	git_reference *resolvedRef;
	int result = git_reference_resolve(&resolvedRef, ref->ref_);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't resolve ref.", result);

	Reference *resolvedRefObj = ref->repository_->wrapReference(resolvedRef);
	return scope.Close(resolvedRefObj->handle_);
}

Handle<Value> Reference::SetTarget(const Arguments &args) {
	HandleScope scope;
	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());
	if(ref->deleted_) {
		THROW_ERROR("This ref has been deleted!");
	}
	
	REQ_ARGS(1);
	
	int result = GIT_ERROR;
	
	if(ref->type_ == GIT_REF_OID) {
		REQ_OID_ARG(0, oidArg);
		result = git_reference_set_oid(ref->ref_, &oidArg);
	}
	else if(ref->type_ == GIT_REF_SYMBOLIC) {
		REQ_STR_ARG(0, targetArg);
		result = git_reference_set_target(ref->ref_, *targetArg);
	}
	
	if(result != GIT_SUCCESS) {
		THROW_GIT_ERROR("Couldn't set target.", result);
	}
	else {
		args.This()->ForceSet(TARGET_PROPERTY, args[0],
				(PropertyAttribute)(ReadOnly | DontDelete));
	}
	
	return scope.Close(True());
}

void Reference::processInitData(void *data) {
	HandleScope scope;
	Handle<Object> jsObject = handle_;

	if(data != NULL) {
		ref_data *refData = static_cast<ref_data*>(data);
		jsObject->Set(NAME_PROPERTY, String::New(refData->name->c_str()),
				(PropertyAttribute)(ReadOnly | DontDelete));

		type_ = refData->type;
		jsObject->Set(TYPE_PROPERTY, Integer::New(refData->type),
				(PropertyAttribute)(ReadOnly | DontDelete));

		jsObject->Set(TARGET_PROPERTY, String::New(refData->target->c_str()),
				(PropertyAttribute)(ReadOnly | DontDelete));

		delete refData->name;
		delete refData->target;
		delete refData;
	}
	else {
		jsObject->Set(NAME_PROPERTY, Null(),
				(PropertyAttribute)(ReadOnly | DontDelete));
		jsObject->Set(TYPE_PROPERTY, Null(),
				(PropertyAttribute)(ReadOnly | DontDelete));
		jsObject->Set(TARGET_PROPERTY, Null(),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}
}

void* Reference::loadInitData() {
	ref_data *data = new ref_data;

	repository_->lockRepository();

	data->name = new std::string(git_reference_name(ref_));
	data->type = git_reference_type(ref_);

	if(data->type == GIT_REF_OID) {
		char id[40];
		git_oid_fmt(id, git_reference_oid(ref_));
		data->target = new std::string(id, 40);
	}
	else if(data->type == GIT_REF_SYMBOLIC) {
		data->target = new std::string(git_reference_target(ref_));
	}

	repository_->unlockRepository();
	return data;
}

} // namespace gitteh
