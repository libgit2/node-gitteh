#include "tag.h"

namespace gitteh {

Persistent<FunctionTemplate> Tag::constructor_template;

void Tag::Init(Handle<Object>) {
	HandleScope scope;

	Handle<FunctionTemplate> t = FunctionTemplate::New(New);
	constructor_template = Persistent<FunctionTemplate>::New(t);
	constructor_template->SetClassName(String::NewSymbol("Tag"));
	t->InstanceTemplate()->SetInternalFieldCount(1);
}

Handle<Value> Tag::New(const Arguments& args) {
	HandleScope scope;

	REQ_ARGS(1);
	REQ_EXT_ARG(0, theTag);

	Tag *tag = new Tag();
	tag->tag_ = static_cast<git_tag*>(theTag->Value());

	char *oidStr = git_oid_allocfmt(git_tag_id(tag->tag_));
	args.This()->Set(String::New("id"), String::New(oidStr), ReadOnly);

	args.This()->Set(String::New("name"), String::New(git_tag_name(tag->tag_)));
	args.This()->Set(String::New("message"), String::New(git_tag_message(tag->tag_)));

	git_signature *tagger = const_cast<git_signature*>(git_tag_tagger(tag->tag_));
	CREATE_PERSON_OBJ(taggerObj, tagger);
	args.This()->Set(String::New("tagger"), taggerObj);

	char *targetOidStr = git_oid_allocfmt(git_object_id(const_cast<git_object*>(git_tag_target(tag->tag_))));
	args.This()->Set(String::New("targetId"), String::New(targetOidStr));
	args.This()->Set(String::New("targetType"), String::New(git_object_type2string(git_tag_type(tag->tag_))));

	tag->Wrap(args.This());
	return args.This();
}

} // namespace gitteh
