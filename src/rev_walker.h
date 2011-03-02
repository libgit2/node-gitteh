#ifndef GITTEH_REV_WALKER_H
#define GITTEH_REV_WALKER_H

#include "gitteh.h"
#include "repository.h"

class RevWalker : public ObjectWrap {
public:
	static Persistent<FunctionTemplate> constructor_template;
	static void Init(Handle<Object>);

	~RevWalker();

	Repository *repo_;

protected:
	static Handle<Value> New(const Arguments&);

	static Handle<Value> Push(const Arguments&);
	static Handle<Value> Hide(const Arguments&);
	static Handle<Value> Next(const Arguments&);

	git_revwalk *walker_;
};

#endif // GITTEH_REV_WALKER_H

