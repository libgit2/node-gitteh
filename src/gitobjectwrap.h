#ifndef GITTEH_TS_OBJECTWRAP_H
#define GITTEH_TS_OBJECTWRAP_H

#include "gitteh.h"
#include <errno.h>

namespace gitteh {

template<class T>
class GitObjectWrap : public ObjectWrap {
public:
	GitObjectWrap() : ObjectWrap() {
		CREATE_MUTEX(gatekeeperLock_);
		initialized_ = 0;
		data_ = NULL;
	}

	bool isInitialized() {
		LOCK_MUTEX(gatekeeperLock_);
		bool initialized = initialized_;
		UNLOCK_MUTEX(gatekeeperLock_);

		return initialized;
	}

	int waitForInitialization() {
		bool needInitialization = false;
		LOCK_MUTEX(gatekeeperLock_);
		if(!initialized_) {
			needInitialization = true;
			CREATE_MUTEX(initLock_);
			LOCK_MUTEX(initLock_);
		}
		UNLOCK_MUTEX(gatekeeperLock_);

		if(needInitialization) {
			int result = GIT_SUCCESS;
			void* data = loadInitData(&result);

			LOCK_MUTEX(gatekeeperLock_);
			data_ = data;
			UNLOCK_MUTEX(gatekeeperLock_);

			UNLOCK_MUTEX(initLock_);

			if(result != GIT_SUCCESS) {
				return result;
			}
		}

		//LOCK_MUTEX(initLock_);
		// TODO: abstract this out in thread.h.
		// Seems that EIO only maintains 1 thread per core by default, or Node
		// does this.. I don't know or care, all I know is using mutexes every
		// where makes Node a sad panda, timers don't work and shit. Instead of
		// blocking hardcore, I'll just spin in a loop trying to lock mutex and
		// sleep for a bit when I can't, effectively yielding the thread.
		while(pthread_mutex_trylock(&initLock_) == EBUSY) {
			usleep(1000);
		}
		UNLOCK_MUTEX(initLock_);

		return GIT_SUCCESS;
	}

	void syncInitialize(void *data) {
		LOCK_MUTEX(gatekeeperLock_);
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	// This should only be called from main thread. When one or more requests
	// are waiting for the same commit to be loaded into memory, whichever one
	// gets out of the blocking waitForInitialization() first will call this on
	// the main thread to ensure the data that was loaded is actually put into
	// the JS object. All the requests will call this, but only the first one
	// will actually make anything meaningful happen.
	void ensureInitDone() {
		LOCK_MUTEX(gatekeeperLock_);
		if(!initialized_) {
			Handle<Value> constructorArgs[1] = { External::New(this) };
			T::constructor_template->GetFunction()->NewInstance(1, constructorArgs);

			processInitData(data_);
			initialized_ = true;
			data_ = NULL;
		}
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	virtual void setOwner(void *owner) = 0;

protected:
	// This is implemented by actual object classes. Implements MUST free all
	// resources allocated by the data on the heap.
	virtual void processInitData(void *data) = 0;
	virtual void* loadInitData(int *result) = 0;

private:
	bool initialized_;
	gitteh_lock gatekeeperLock_;
	gitteh_lock initLock_;
	void *data_;
};

} // namespace gitteh

#endif // GITTEH_TS_OBJECTWRAP_H
