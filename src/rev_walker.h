#ifndef GITTEH_REV_WALKER_H
#define GITTEH_REV_WALKER_H

#include "gitteh.h"

namespace gitteh {

class Repository;

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
	static Handle<Value> Sort(const Arguments&);
	static Handle<Value> Reset(const Arguments&);

	git_revwalk *walker_;
};

} // namespace gitteh

#endif // GITTEH_REV_WALKER_H

