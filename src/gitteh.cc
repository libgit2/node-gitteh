#include "gitteh.h"

#include "commit.h"
#include "tree.h"
#include "tree_entry.h"
#include "rawobj.h"
#include "repository.h"
#include "index.h"
#include "index_entry.h"
#include "tag.h"
#include "rev_walker.h"
#include "error.h"
#include "ref.h"

namespace gitteh {

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Repository::Init(target);
	RawObject::Init(target);
	Commit::Init(target);
	Tree::Init(target);
	TreeEntry::Init(target);
	Index::Init(target);
	IndexEntry::Init(target);
	Tag::Init(target);
	RevWalker::Init(target);
	Reference::Init(target);

	ErrorInit(target);
}

} // namespace gitteh
