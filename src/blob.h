#ifndef GITTEH_BLOB_H
#define GITTEH_BLOB_H

#include "gitteh.h"

namespace gitteh {
	namespace Blob {
		void Init(Handle<Object>);
		Handle<Object> Create(git_blob*);
	}
};

#endif // GITTEH_BLOB_H
