#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "repository.h"

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

class Tree : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);
	~Tree();

protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> IdGetter(Local<String> property, const AccessorInfo& info);
	static Handle<Value> LengthGetter(Local<String> property, const AccessorInfo& info);
	static Handle<Value> IndexHandler(uint32_t index, const AccessorInfo& info);

	git_tree *tree_;
	Repository *repo_;
};

#endif	// GITTEH_TREE_H
