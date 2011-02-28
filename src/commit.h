#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "repository.h"
#include "tree.h"

#define COMMIT_ID_SYMBOL String::NewSymbol("id")
#define COMMIT_MESSAGE_SHORT_SYMBOL String::NewSymbol("messageShort")
#define COMMIT_MESSAGE_SYMBOL String::NewSymbol("message")
#define COMMIT_TIME_SYMBOL String::NewSymbol("time")
#define COMMIT_AUTHOR_SYMBOL String::NewSymbol("author")
#define COMMIT_COMMITTER_SYMBOL String::NewSymbol("committer")
#define COMMIT_TREE_SYMBOL String::NewSymbol("tree")

class Commit : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);
	~Commit();

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> TreeGetter(Local<String>, const AccessorInfo&);

	Repository *repo_;
	git_commit *commit_;
};

#endif // GITTEH_COMMIT_H
