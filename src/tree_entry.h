#ifndef GITTEH_TREE_ENTRY_H
#define GITTEH_TREE_ENTRY_H

#include "gitteh.h"
#include "tree.h"

class TreeEntry : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);

protected:
	static Handle<Value> New(const Arguments& args);

	git_tree_entry *entry_;
	Tree *tree_;
};

#endif // GITTEH_TREE_ENTRY_H
