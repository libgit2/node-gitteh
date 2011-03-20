# TODO

* Delete support.
* Integrity tests, make sure bindings don't choke or segfault on really bad data.
* Actual blob support. Not sure if this is really necessary, as the raw object is already exposing a Buffer to manipulate contents, and then Node provides plenty of functionality to load data from files etc.
* Revisit index support, expand and add tests.
* Possibly implement custom backend support, allowing JS callbacks to provide a custom git backend.
* Check for memory leaks
* Support for ref deletion.
* Implement ref packing method call, without breaking shit.
