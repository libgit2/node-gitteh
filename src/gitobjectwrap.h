#ifndef GITTEH_TS_OBJECTWRAP_H
#define GITTEH_TS_OBJECTWRAP_H

#include "gitteh.h"
#include <errno.h>

namespace gitteh {

class GitObjectWrap : public ObjectWrap {
public:
	GitObjectWrap() : ObjectWrap() {
		CREATE_MUTEX(gatekeeperLock_);
		initialized_ = 0;
		initInterest_ = 0;
		data_ = NULL;
	}

	// Shortcut, if this is a newly allocated object for a newly created Git
	// object, then there's not going to be any threads fighting for access,
	// we just mark it as initialized straight up, no need to even use a mutex.
	void forceInitialized() {
		processInitData(NULL);
		initialized_ = true;
	}

	bool isInitialized() {
		LOCK_MUTEX(gatekeeperLock_);
		bool initialized = initialized_;
		UNLOCK_MUTEX(gatekeeperLock_);

		return initialized;
	}

	void registerInitInterest() {
		LOCK_MUTEX(gatekeeperLock_);
		Ref();
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	void removeInitInterest() {
		LOCK_MUTEX(gatekeeperLock_);
		Unref();
		initInterest_--;

		if(initInterest_ == 0) {
			DESTROY_MUTEX(initLock_);
		}
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	void waitForInitialization() {
		bool needInitialization = false;
		LOCK_MUTEX(gatekeeperLock_);
		if(!initInterest_++) {
			needInitialization = true;
			CREATE_MUTEX(initLock_);
			LOCK_MUTEX(initLock_);
		}
		UNLOCK_MUTEX(gatekeeperLock_);

		if(needInitialization) {
			void* data = loadInitData();
			initializationDone(data);
			return;
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
	}

	void syncInitialize(void *data) {
		LOCK_MUTEX(gatekeeperLock_);
		UNLOCK_MUTEX(gatekeeperLock_);
	}

	// Signalled by the thread that is building the commit data that the data
	// is now done and JS object can be finalized.
	void initializationDone(void *data) {
		LOCK_MUTEX(gatekeeperLock_);
		data_ = data;
		UNLOCK_MUTEX(gatekeeperLock_);

		UNLOCK_MUTEX(initLock_);
	}

	// This should only be called from main thread. When one or more requests
	// are waiting for the same commit to be loaded into memory, whichever one
	// gets out of the blocking waitForInitialization() first will call this on
	// the main thread to ensure the data that was loaded is actually put into
	// the JS object. All the requests will call this, but only the first one
	// will actually make anything meaningful happen.
	void ensureInitDone() {
		// FIXME? don't think any locking is necessary here as this is ONLY
		// called from main thread.
		LOCK_MUTEX(gatekeeperLock_);
		if(data_ != NULL) {
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
	virtual void* loadInitData() = 0;

private:
	bool initialized_;
	int initInterest_;
	gitteh_lock gatekeeperLock_;
	gitteh_lock initLock_;
	void *data_;
};

} // namespace gitteh

#endif // GITTEH_TS_OBJECTWRAP_H
