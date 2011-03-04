#ifndef GITTEH_TREE_ENTRY_H
#define GITTEH_TREE_ENTRY_H

#include "gitteh.h"

namespace gitteh {

class Tree;

class TreeEntry : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);
	~TreeEntry();

	Tree *tree_;

protected:
	static Handle<Value> New(const Arguments& args);

	static Handle<Value> SetterHandler(Local<String>, Local<Value>, const AccessorInfo&);

	git_tree_entry *entry_;
};

} // namespace gitteh

#endif // GITTEH_TREE_ENTRY_H
