#ifndef GITTEH_TREE_H
#define GITTEH_TREE_H

#include "gitteh.h"
#include "git_object.h"

namespace gitteh {
	class Tree : public GitObject {
	public:
		Tree(git_tree*);
		~Tree();
		static Persistent<FunctionTemplate> constructor_template;
		static void Init(Handle<Object>);
	protected:
		static Handle<Value> New(const Arguments&);
	private:
		git_tree *tree_;
	};
} // namespace gitteh

#endif	// GITTEH_TREE_H
