# Gitteh

**NOTE: Gitteh is currently undergoing heavy refactoring/rewriting to both not suck and be compatible with v0.17.0. Watch this space!**

## What?

Node bindings to the excellent [libgit2](http://libgit2.github.com) C library. The bindings cover *most* of the libgit2 API, however I took some liberties. For example...

* There's no notion of "oids" like in other libraries, all object ids are referenced by their 40 character SHA1 string.
* I didn't just write some quick bridge code to access libgit2 stuff, I took the time to distill the libgit2 API into an organized, intuitive set of objects you can work with.
* I avoided calling into C code where possible. This means that when you're composing a new commit for example, you can set most of the properties of the commit on a JS object as standard properties, then call save() when you're ready. Calling into C++ code is inherently expensive with V8.
* Although some of it is missing from the repo at the moment, I've written lots of little stress tests to make sure this library doesn't go ahead and segfault your server. Libgit2 isn't thread-safe at all. Hey, no need to thank me, it's all part of the job (it's why I get to wear a cape, and you don't). Essentially this means that you, libgit2, Node, and V8's garbage compiler can all play in the sandpit nicely together.
* I didn't bother wrapping blob API functions, since all they do is offer helper methods to load files into blobs, save blobs out to files etc. You can do all this with a RawObject, it exposes a Buffer with the contents of any objects in a git repo. Node has cooler filesystem stuff anyway.
* No animals were harmed during development, except that one little hamst... you know what? Never mind.

## Why?

Why not? Libgit2 is an excellent way to work with a Git repository in a well-defined and speedy manner. 

Or you could, you know, manually execute `git` CLI commands and parse stdout. Have fun with that. 

## How?

Installation can be done via npm.

	npm install gitteh
	
Currently, installing Gitteh via NPM will mean that Gitteh will compile it's own
bundled version of libgit2, even if it's already installed on your system. I've
done this because libgit2 is releasing quite rapidly and each release is breaking
functionality. So while libgit2 is in alpha state, I will be forcing a specific
version of libgit2 with each release of Gitteh.

The current version of Gitteh requires libgit2 v0.11.0.

[Documentation can be found here.](http://libgit2.github.com/node-gitteh/docs/index.html). You should also check out the examples in the examples/ dir in the repo.

## Halp?

Actually, yes. I'd really appreciate any contributions on this project. Check out the TODO to see what still needs to be done, then get in contact with me BEFORE you start coding so I can make sure you're not doubling up work that's already under wraps :)
