#ifndef GITTEH_BLOB_H
#define GITTEH_BLOB_H

#include "gitteh.h"
#include "git_object.h"

namespace gitteh {
	class Blob : public GitObject {
	public:
		Blob(git_blob*);
		~Blob();
		static Persistent<FunctionTemplate> constructor_template;
		static void Init(Handle<Object>);
	protected:
		static Handle<Value> New(const Arguments&);
	private:
		git_blob *blob_;
	};
} // namespace gitteh

#endif // GITTEH_BLOB_H
