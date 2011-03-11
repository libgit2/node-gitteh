#ifndef GITTEH_TS_OBJECTWRAP_H
#define GITTEH_TS_OBJECTWRAP_H

#include "gitteh.h"
#include <errno.h>

namespace gitteh {

class ThreadSafeObjectWrap : public ObjectWrap {
public:
	ThreadSafeObjectWrap() : ObjectWrap() {
		CREATE_MUTEX(gatekeeperLock_);
		initialized_ = 0;
		initInterest_ = 0;
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

	bool waitForInitialization() {
		bool needInitialization = false;
		LOCK_MUTEX(gatekeeperLock_);
		if(!initInterest_++) {
			needInitialization = true;
			CREATE_MUTEX(initLock_);
			LOCK_MUTEX(initLock_);
		}
		UNLOCK_MUTEX(gatekeeperLock_);

		if(needInitialization) {
			return true;
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

	void initializationDone() {
		LOCK_MUTEX(gatekeeperLock_);
		initialized_ = true;
		UNLOCK_MUTEX(gatekeeperLock_);

		UNLOCK_MUTEX(initLock_);
	}


private:
	bool initialized_;
	int initInterest_;
	gitteh_lock gatekeeperLock_;
	gitteh_lock initLock_;
};

} // namespace gitteh

#endif // GITTEH_TS_OBJECTWRAP_H
