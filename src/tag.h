#ifndef GITTEH_TAG_H
#define GITTEH_TAG_H

#include "gitteh.h"
#include "gitobjectwrap_new.h"

namespace gitteh {

class Repository;
struct tag_data;

class Tag : public WrappedGitObject<Tag, git_tag>  {
public:
	static Persistent<FunctionTemplate> constructor_template;

	static void Init(Handle<Object>);
	static Handle<Value> SaveObject(Handle<Object>, Repository*, Handle<Value>, bool);

	Tag(git_tag*);
	~Tag();
	void setOwner(void*);

	Repository *repository_;

protected:
	static Handle<Value> New(const Arguments&);
	static Handle<Value> Save(const Arguments&);

	void processInitData();
	int doInit();

	git_tag *tag_;

private:
	static int EIO_Save(eio_req*);
	static int EIO_AfterSave(eio_req*);

	tag_data *initData_;
};

} // namespace gitteh

#endif	// GITTEH_TAG_H
