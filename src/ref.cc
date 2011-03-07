#include "ref.h"
#include "repository.h"

#define NAME_PROPERTY String::NewSymbol("name")
#define TYPE_PROPERTY String::NewSymbol("type")
#define TARGET_PROPERTY String::NewSymbol("target")

namespace gitteh {

Persistent<FunctionTemplate> Reference::constructor_template;

void Reference::Init(Handle<Object> target) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::New("Reference"));
	constructor_template->InstanceTemplate()->SetInternalFieldCount(1);

	NODE_SET_PROTOTYPE_METHOD(t, "rename", Rename);
	NODE_SET_PROTOTYPE_METHOD(t, "delete", Delete);
	NODE_SET_PROTOTYPE_METHOD(t, "resolve", Resolve);

	NODE_DEFINE_CONSTANT(target, GIT_REF_OID);
	NODE_DEFINE_CONSTANT(target, GIT_REF_SYMBOLIC);
}

Handle<Value> Reference::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, refArg);

	Reference *ref = new Reference();
	ref->ref_ = static_cast<git_reference*>(refArg->Value());
	ref->Wrap(args.This());

	git_rtype type = git_reference_type(ref->ref_);

	args.This()->Set(NAME_PROPERTY, String::New(git_reference_name(ref->ref_)),
			(PropertyAttribute)(ReadOnly | DontDelete));

	args.This()->Set(TYPE_PROPERTY, Integer::New(type),
			(PropertyAttribute)(ReadOnly | DontDelete));

	if(type == GIT_REF_OID) {
		const char *oidStr = git_oid_allocfmt(git_reference_oid(ref->ref_));
		args.This()->Set(TARGET_PROPERTY, String::New(oidStr),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}
	else if(type == GIT_REF_SYMBOLIC) {
		args.This()->Set(TARGET_PROPERTY, String::New(git_reference_target(ref->ref_)),
				(PropertyAttribute)(ReadOnly | DontDelete));
	}

	return scope.Close(args.This());
}

Handle<Value> Reference::Rename(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_STR_ARG(0, newNameArg);

	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());

	int result = git_reference_rename(ref->ref_, *newNameArg);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't rename ref.", result);

	return scope.Close(Undefined());
}

Handle<Value> Reference::Delete(const Arguments &args) {
	HandleScope scope;

// TODO:
	return scope.Close(Undefined());
}

Handle<Value> Reference::Resolve(const Arguments &args) {
	HandleScope scope;

	Reference *ref = ObjectWrap::Unwrap<Reference>(args.This());

	git_reference *resolvedRef;
	int result = git_reference_resolve(&resolvedRef, ref->ref_);
	if(result != GIT_SUCCESS)
		THROW_GIT_ERROR("Couldn't resolve ref.", result);

	Reference *resolvedRefObj = ref->repository_->wrapReference(resolvedRef);
	return scope.Close(resolvedRefObj->handle_);
}

} // namespace gitteh
