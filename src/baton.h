#ifndef GITTEH_BATON_H
#define GITTEH_BATON_H

#include <v8.h>
#include <node.h>
#include <git2.h>
#include <string>

using namespace v8;
using std::string;

namespace gitteh {
/**
	A Baton class specifically for libgit2 work.
	Users of this Baton are expected to copy a git_error into this class.
*/
class Baton {
public:
	uv_work_t req;
	Persistent<Function> callback;
	int errorCode;
	string errorString;

	Baton();
	~Baton();
	void setCallback(Handle<Value> val);
	bool isErrored();
	void setError(const git_error *err);
	// Creates an Exception for this error state Baton. DON'T CALL OUTSIDE OF
	// V8 MAIN THREAD! :)
	Handle<Object> createV8Error();
	void defaultCallback();
};

}; // namespace gitteh

#endif // GITTEH_BATON_H
