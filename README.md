# Gitteh `v0.17.1`

## What?

Node bindings to the excellent [libgit2](http://libgit2.github.com) C library. Right now, the bindings cover read only access to raw objects (trees/commits/blobs/tags/references), and basic access to remotes (including basic cloning/fetching support).

Gitteh aims to:

* Be simple and convenient
* Be as performant as possible - avoids calling into underlying C library as much as possible.
* Be safe for use in Node's threadpool (read: shouldn't segfault your app)

## Why?

There's a few libraries out there that wrap git cli commands, parsing the output and such. This is a perfectly acceptable solution. Node-gitteh provides first-class support to work with a git repository on a low level, and does not require git.git (and its myriad of dependencies) to be installed in the server environment.

## Installation

Gitteh has been written for Node v0.6.x and higher.

Installation can be done via npm.

	npm install gitteh

Currently, installing gitteh will require CMake 2.6+ to be present on your machine. Ideally gitteh should be able to build the underlying libgit2 dependency using `gyp`, as it ships with all recent versions of npm. If you need this (for example for deployment to Heroku), feel free to contact me, or better yet, contribute it yourself!

[Documentation can be found here](http://libgit2.github.com/node-gitteh/docs/index.html). You should also check out the examples in the examples/ dir in the repo.

## License

Gitteh is MIT licensed to be consistent with libgit2.

## Contributing

Contributions are very welcome. Please feel free to fork this project and hack on it. Go ahead and check out the issues tab to see what needs to be done! Go on! Do it!