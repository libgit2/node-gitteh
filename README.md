# Gitteh

## What?

Native bindings to libgit2. The bindings aren't quite 1:1 functions though. I wrapped most of the functionality up into neat object boundaries. That is, creating a commit is done from a Repository, then managing that commit is done from a Commit object.

## Why?

Why not? Libgit2 is pretty much the de-facto standard for working with a Git repo programmatically. The alternative is to use the git CLI and parse output manually. That's nasty.

## How?

You need to [install libgit2](http://libgit2.github.com/#install) first.

Installation can be done via npm.

	npm install gitteh

Documentation can be found at [http://samcday.github.com/node-gitteh](http://samcday.github.com/node-gitteh).

## Can I has halp?

Actually, yes. I'd really appreciate any contributions on this project. Check out the TODO to see what still needs to be done, then get in contact with me BEFORE you start coding so I can make sure you're not doubling up work that's already under wraps :)
