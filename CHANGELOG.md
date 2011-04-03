## v0.1.0 - 03/04/11
* Heavy refactors to bring gitteh up to date with libgit2 0.11.0.
* Commits no longer have methods to retrieve tree/parents. Instead, they are just sha1 string properties on the object.
* Trees no longer have getEntry() method, all tree entries are added to an array in the tree.entries property.
* Tree writing is currently unsupported (as it's been broken in libgit2), next version will allow writing an index as a tree.
* Index file support, git index can now be manipulated - add entries/remove them/query them/whatever!
* Ref deletion is now supported.
* Ref packing is supported. This is a fairly advanced feature and required more work than it was worth. Any issues reported on this will be followed up ASAP.
* Full documentation written for the bindings. Huzzah.

## v0.0.3 - 20/03/11
* Fixed up some pretty serious memory leaks stemming from how git_oid and git_signatures were being handled.

## v0.0.2 - 20/03/11
* Minor fix to support 64-bit compiles (thanks TooTallNate).
* Improved build system, will now automatically checkout libgit2 and build it if it's not already present on system.

## v0.0.1 - 18/03/11
* Initial release.
