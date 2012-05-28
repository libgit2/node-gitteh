module.exports = Gitteh = require "../build/Debug/gitteh"

{Repository, Tree, Blob} = Gitteh

immutable = (obj, src) ->
	return o = {
		set: (name, target = name) ->
			if typeof src[name] is "array"
				return Object.defineProperty obj, target,
					get: () -> src[val].slice(0)
					configurable: false
					enumerable: true
			Object.defineProperty obj, target, 
				value: src[name]
				writable: false
				configurable: false
				enumerable: true
			return o
	}

Signature = (obj) ->
	immutable(@, obj)
		.set("name")
		.set("email")
		.set("time")
		.set("offset")
	return @

Gitteh.Commit = Commit = (@repository, obj) ->
	obj.author = new Signature obj.author
	obj.committer = new Signature obj.committer
	immutable(@, obj)
		.set("id")
		.set("tree", "treeId")
		.set("parents")
		.set("message")
		.set("messageEncoding")
		.set("author")
		.set("committer")
	@tree = (cb) =>
		@repository.tree obj.tree, cb
	return @

Gitteh.Tree = Tree = (@repository, obj) ->
	obj._entries = obj.entries
	obj.entries = []
	for origEntry in obj._entries
		obj.entries.push entry = {}
		immutable(entry, origEntry)
			.set("id")
			.set("name")
			.set("type")
			.set("attributes")
	immutable(@, obj)
		.set("id")
		.set("entries")
	return @

Gitteh.Blob = Blob = (@repository, obj) ->
	immutable(@, obj)
		.set("id")
		.set("data")
	return @

Gitteh.Tag = Tag = (@repository, obj) ->
	obj.tagger = new Signature obj.tagger
	immutable(@, obj)
		.set("id")
		.set("name")
		.set("message")
		.set("tagger")
		.set("target", "targetId")
		.set("type")
	return @
oidRegex = /^[a-zA-Z0-9]{0,40}$/
checkOid = (str, allowLookup = true) ->
	throw new TypeError "OID should be a string" if typeof str isnt "string"
	throw new TypeError "Invalid OID" if not oidRegex.test str
	throw new Error "OID is too short" if str.length < Gitteh.minOidLength
	throw new TypeError "Invalid OID" if not allowLookup and str.length isnt 40

wrap = (clazz, fn, prototype, newFn) ->
	override = if prototype then clazz.prototype else clazz
	orig = override[fn]

	override[fn] = ->
		shadowed = if prototype then orig.bind @ else orig
		newFn.apply @, [shadowed].concat Array.prototype.slice.call arguments

wrap Gitteh, "openRepository", false, (shadowed, path, cb) ->
	shadowed path, cb

wrap Gitteh, "initRepository", false, (shadowed, path, bare, cb) ->
	if typeof bare is "function"
		cb = bare
		bare = false 

	shadowed path, bare, cb

wrap Repository, "exists", true, (shadowed, oid, cb) ->
	checkOid oid, false
	shadowed oid, cb

wrap Repository, "object", true, (shadowed, oid, cb) ->
	# TODO: change this to true when we support object_lookup_prefix
	checkOid oid
	shadowed oid, cb

# OBJECT LOOKUP METHODS
# We do a little bit of extra due dilligence here, ensuring that a call to 
# commit() for a tree OID fails gracefully-ish.

wrapObjectCallback = (cb, oid, expectedType) ->
	(err, obj) ->
		return cb err if err?
		if obj not instanceof expectedType
			return cb new TypeError "#{oid} is not a #{expectedType.prototype.constructor.name}"
		cb err, obj
Repository.prototype.commit = (oid, cb) ->
	@object oid, (err, obj) =>
		return cb err if err?
		cb null, new Commit @, obj

Repository.prototype.tree = (oid, cb) ->
	@object oid, (err, obj) =>
		return cb err if err?
		cb null, new Tree @, obj

Repository.prototype.blob = (oid, cb) ->
	@object oid, (err, obj) =>
		return cb err if err?
		cb null, new Blob @, obj

Repository.prototype.tag = (oid, cb) ->
	@object oid, (err, obj) =>
		return cb err if err?
		cb null, new Tag @, obj

wrap Repository, "reference", true, (shadowed, name, resolve, cb) ->
	if typeof resolve is "function"
		cb = resolve
		resolve = false
	shadowed name, resolve, cb

Repository.prototype.ref = (name, resolve, cb) ->
	@reference name, resolve, cb
