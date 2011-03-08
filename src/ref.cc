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
	ref->ref_ = static_cast<git_reference*>(refArg->Value());
	ref->type_ = git_reference_type(ref->ref_);
	ref->Wrap(args.This());

	args.This()->Set(NAME_PROPERTY, String::New(git_reference_name(ref->ref_)),
			(PropertyAttribute)(ReadOnly | DontDelete));

	args.This()->Set(TYPE_PROPERTY, Integer::New(ref->type_),
			(PropertyAttribute)(ReadOnly | DontDelete));

	if(ref->type_ == GIT_REF_OID) {
		const char *oidStr = git_oid_allocfmt(git_reference_oid(ref->ref_));
		args.This()->Set(TARGET_PROPERTY, String::New(oidStr),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}
	else if(ref->type_ == GIT_REF_SYMBOLIC) {
		args.This()->Set(TARGET_PROPERTY, String::New(git_reference_target(ref->ref_)),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}

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

} // namespace gitteh
