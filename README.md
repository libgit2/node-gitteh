# Gitteh `v0.17.2`

[![build status](https://secure.travis-ci.org/libgit2/node-gitteh.png)](http://travis-ci.org/libgit2/node-gitteh)

## Gitteh needs a new daddy (or mommy): See [Issue #68](https://github.com/libgit2/node-gitteh/issues/68).

## Installation

Gitteh requires `node 0.8`, `CMake 2.6` and `gcc`. Installation via NPM:

	npm install gitteh

## What?

Node bindings to the excellent [libgit2](http://libgit2.github.com) C library. Right now, the bindings cover read only access to raw objects (trees/commits/blobs/tags/references), and basic access to remotes (including basic cloning/fetching support).

Gitteh aims to:

* Be simple and convenient
* Be as performant as possible - avoids calling into underlying C library as much as possible.
* Be safe for use in Node's threadpool (read: shouldn't segfault your app)

## Why?

There's a few libraries out there that wrap git cli commands, parsing the output and such. This is a perfectly acceptable solution. Node-gitteh provides first-class support to work with a git repository on a low level, and does not require git.git (and its myriad of dependencies) to be installed in the server environment.

The documentation is currently outdated. We're working on a new version currently for the new 0.17.x version of gitteh. The [Old pre-0.17.x documentation can be found here](http://libgit2.github.com/node-gitteh/docs/index.html). Instead, you should visit [`examples`](https://github.com/libgit2/node-gitteh/tree/master/examples) dir in the repo to see examples of 0.17.x usage.

## License

Gitteh is available under the MIT License. See the LICENSE file for more information.

## Contributing

Contributions are very welcome. Please feel free to fork this project and hack on it. Go ahead and check out the issues tab to see what needs to be done! Go on! Do it!

### Building

Next, you need to install gitteh's dependencies and build it. In gitteh's root directory, run:

    npm install
    node preinstall.js
    node install.js

### Installing Manually

Copy the parent directory of the now built module to the node_modules directory of your project. Include the module using:
    gitteh = require('gitteh');
