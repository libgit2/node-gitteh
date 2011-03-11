#ifndef GITTEH_THREAD_H
#define GITTEH_THREAD_H

// I honestly have no clue what the fuck I'm doing. Libgit2 doesn't seem to be
// thread-safe, so I need an object to synchronize with to prevent crazy shit
// happening when libeio kicks in and I'm doing libgit2 stuff in random threads.

// I must be missing something, I've never really worked with threads in C++ 
// before, and I understand that they're not a first class feature of the
// current C++ standard, but I figured libeio would have some form of lock in
// place, apparently not o.O. Apparently libeio relies heavily on POSIX threads
// though, so one would think I should be ok to use pthread_mutex_lock and 
// pthread_mutex_unlock? Just in case though I'm setting up macros for them here
// so I can swap them out or write compiler/platform specific variants later.

typedef pthread_mutex_t gitteh_lock;
#define CREATE_MUTEX(LOCK)													\
	pthread_mutex_init (&LOCK, NULL);

#define DESTROY_MUTEX(LOCK)													\
	pthread_mutex_destroy(&LOCK);

#define LOCK_MUTEX(LOCK)													\
	pthread_mutex_lock(&LOCK);
	
#define UNLOCK_MUTEX(LOCK)													\
	pthread_mutex_unlock(&LOCK)
	

#endif // GITTEH_THREAD_H
