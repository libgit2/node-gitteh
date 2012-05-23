Gitteh = require "../build/Release/gitteh"

{Repository, Commit} = Gitteh

wrap = (clazz, fn, prototype, newFn) ->
	orig = if prototype then clazz.prototype[fn] else clazz[fn]
	clazz[fn] = ->
		shadowed = if prototype then orig.bind @ else orig
		newFn.apply @, [shadowed].concat Array.prototype.slice.call arguments

module.exports = Gitteh

wrap Gitteh, "openRepository", false, (shadowed, path, cb) ->
	shadowed path, cb

wrap Gitteh, "initRepository", false, (shadowed, path, bare, cb) ->
	if typeof bare is "function"
		cb = bare
		bare = false 

	shadowed path, bare, cb

wrap Repository, "exists", true, (shadowed, oid, cb) ->
	shadowed oid, cb

wrap Repository, "getCommit", true, (shadowed, oid, cb) ->
	shadowed oid, wrappedCb
