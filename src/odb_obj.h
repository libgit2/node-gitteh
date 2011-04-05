#ifndef GITTEH_ODB_OBJ_H
#define GITTEH_ODB_OBJ_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

class ObjectDatabase;

class ODBObject : public WrappedGitObject<ObjectDatabase, ODBObject, git_odb_object> {
public:

protected:

private:

};

} // namespace gitteh

#endif //GITTEH_ODB_OBJ_H
