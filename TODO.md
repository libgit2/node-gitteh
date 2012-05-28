TODO:

* Lookup lightweight/annotated tag by name.


# (Old) TODO - probably still relevant

* Integrity tests, make sure bindings don't choke or segfault on really bad data.
* Possibly implement custom backend support, allowing JS callbacks to provide a custom git backend.
* Check for memory leaks
* Cache raw objects properly, so two requests for the same oid don't result in different objects.
* Maybe add convenience methods to all existing wrapped objects to get the raw object equivalent of them.
* Error handling in the EIO initialization stuff for objects. Not sure if something can go wrong there, but better safe than sorry.
* Tests for initializing a new repository, bare or workingdir.
* Stress test suite.
* Perf tests.
* See if we can remove the lock on repository for some serious speed.
* Make sure all create/get stuff in repo is returning local copies of handles.
* Update Index to not use getters/setters for index modification. Instead, work with an array like tree entries.