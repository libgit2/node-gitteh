Gitteh = require "../build/Debug/gitteh"

{Repository, Commit, Tree} = Gitteh

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

wrap Repository, "object", true, (shadowed, oid, cb) ->
	shadowed oid, cb

Repository.prototype.commit = (oid, cb) ->
	@object oid, (err, commit) ->
		return cb err if err
		return cb new TypeError "#{oid} is not a Commit!" if commit not instanceof Commit
		cb err, commit
Repository.prototype.tree = (oid, cb) ->
	@object oid, (err, tree) ->
		return cb err if err
		return cb new TypeError "#{oid} is not a Tree!" if tree not instanceof Tree
		cb err, tree
Repository.prototype.blob = (oid, cb) ->
	@object oid, cb
