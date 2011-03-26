#ifndef GITTEH_TREE_ENTRY_H
#define GITTEH_TREE_ENTRY_H

#include "gitteh.h"
#include "gitobjectwrap.h"

namespace gitteh {

class Tree;

class TreeEntry : public GitObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object> target);
	~TreeEntry();

	void setOwner(void*);


protected:
	static Handle<Value> New(const Arguments& args);
	static Handle<Value> SetterHandler(Local<String>, Local<Value>, const AccessorInfo&);

	void processInitData(void*);
	void *loadInitData();

	git_tree_entry *entry_;

private:
	Tree *tree_;
};

} // namespace gitteh

#endif // GITTEH_TREE_ENTRY_H
