#ifndef GITTEH_REMOTE_H
#define GITTEH_REMOTE_H

#include "gitteh.h"

namespace gitteh {
	class Remote : public ObjectWrap {
	public:
		static Persistent<FunctionTemplate> constructor_template;
		static void Init(Handle<Object>);
		Remote(git_remote*);
		~Remote();
	
	protected:
		static Handle<Value> New(const Arguments&);

	private:
		git_remote *remote_;
	};
};

#endif
