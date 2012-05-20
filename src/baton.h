#ifndef GITTEH_BATON_H
#define GITTEH_BATON_H

#include <v8.h>
#include <node.h>
#include <git2.h>

using namespace v8;

namespace gitteh {
/**
	A Baton class specifically for libgit2 work.
	Users of this Baton are expected to copy a git_error into this class.
*/
class Baton {
public:
	uv_work_t req;
	Persistent<Function> callback;
	git_error error;

	Baton();
	~Baton();
	void setCallback(Handle<Value> val);
	bool isErrored();
	// Creates an Exception for this error state Baton. DON'T CALL OUTSIDE OF
	// V8 MAIN THREAD! :)
	Handle<Object> createV8Error();
};

}; // namespace gitteh

#endif // GITTEH_BATON_H
