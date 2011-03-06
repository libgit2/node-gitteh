# TODO

* Async support.
* Delete support.
* Integrity tests, make sure bindings don't choke or segfault on really bad data.
* Repository init (creation) support.
* Actual blob support. Not sure if this is really necessary, as the raw object is already exposing a Buffer to manipulate contents, and then Node provides plenty of functionality to load data from files etc.
* Revisit index support, expand and add tests.
* Possibly implement custom backend support, allowing JS callbacks to provide a custom git backend.
* Suss out libgit2's ref support. It's not documented at all in their Doxygen atm, but there are mentions of it in rugged, and I can see it in the source code of libgit2.
