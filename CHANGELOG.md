## v0.0.4 - 29/03/11
* Added support for working with git indexes. 
* Fixed issues that could cause asynchronous retrieval of objects to fail miserably.

## v0.0.3 - 20/03/11
* Fixed up some pretty serious memory leaks stemming from how git_oid and git_signatures were being handled.

## v0.0.2 - 20/03/11
* Minor fix to support 64-bit compiles (thanks TooTallNate).
* Improved build system, will now automatically checkout libgit2 and build it if it's not already present on system.

## v0.0.1 - 18/03/11
* Initial release.
