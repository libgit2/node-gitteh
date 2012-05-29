module.exports = Gitteh = require "../build/Debug/gitteh"
{Repository} = Gitteh

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
	@target = (cb) =>
		@repository.object @targetId, @type, cb
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

wrap Repository, "object", true, (shadowed, args..., cb) ->
	[oid, type] = args
	type = "any" if not type
	checkOid oid
	shadowed oid, type, (err, object) =>
		return cb err if err?
		clazz = switch object._type
			when Gitteh.types.commit then Commit
			when Gitteh.types.tree then Tree
			when Gitteh.types.blob then Blob
			when Gitteh.types.tag then Tag
			else undefined
		return cb new TypeError("Unexpected object type") if clazz is undefined
		return cb null, new clazz @, object

Repository.prototype.commit = (oid, cb) ->
	@object oid, "commit", cb

Repository.prototype.tree = (oid, cb) ->
	@object oid, "tree", cb

Repository.prototype.blob = (oid, cb) ->
	@object oid, "blob", cb

Repository.prototype.tag = (oid, cb) ->
	@object oid, "tag", cb

wrap Repository, "reference", true, (shadowed, name, resolve, cb) ->
	if typeof resolve is "function"
		cb = resolve
		resolve = false
	shadowed name, resolve, cb

Repository.prototype.ref = Repository.prototype.reference
