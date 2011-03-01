#include "gitteh.h"

#include "commit.h"
#include "tree.h"
#include "tree_entry.h"
#include "rawobj.h"
#include "odb.h"
#include "repository.h"
#include "index.h"
#include "index_entry.h"

extern "C" void
init(Handle<Object> target) {
	HandleScope scope;
	Repository::Init(target);
	RawObject::Init(target);
	ObjectDatabase::Init(target);
	Commit::Init(target);
	Tree::Init(target);
	TreeEntry::Init(target);
	Index::Init(target);
	IndexEntry::Init(target);
}
