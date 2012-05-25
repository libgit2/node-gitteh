#ifndef GITTEH_COMMIT_H
#define GITTEH_COMMIT_H

#include "gitteh.h"
#include "git_object.h"

namespace gitteh {
	class Commit : public GitObject {
	public:
		static Persistent<FunctionTemplate> constructor_template;
		static void Init(Handle<Object>);
		Commit(git_commit*);
		~Commit();
	protected:
		static Handle<Value> New(const Arguments&);
	private:
		git_commit *commit_;
	};
} // namespace gitteh

#endif // GITTEH_COMMIT_H
