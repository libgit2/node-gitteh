#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "repository.h"
#include <map>

#define TREE_ID_SYMBOL String::NewSymbol("id")
#define TREE_LENGTH_SYMBOL String::NewSymbol("length")

class Tree : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	~Tree();

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> LengthGetter(Local<String>, const AccessorInfo&);
	static Handle<Value> IndexHandler(uint32_t, const AccessorInfo&);

	git_tree *tree_;
	Repository *repo_;
	std::map<int, void*> treeEntryObjects_;
};

#endif	// GITTEH_TREE_H
