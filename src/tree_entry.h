#ifndef GITTEH_TREE_ENTRY_H
#define GITTEH_TREE_ENTRY_H

#include "gitteh.h"

#define TREE_ENTRY_NAME_SYMBOL String::NewSymbol("name")

class TreeEntry : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> NameGetter(Local<String> property, const AccessorInfo& info);

	git_tree_entry *entry_;
};

#endif // GITTEH_TREE_ENTRY_H
