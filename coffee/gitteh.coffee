bindings = require "../build/Debug/gitteh"
{minOidLength, types, NativeRepository} = bindings
args = require "./args"

module.exports = Gitteh = {}

oidRegex = /^[a-zA-Z0-9]{0,40}$/
args.validators.oid = (val) ->
	return false if typeof val isnt "string"
	return false if not oidRegex.test val
	return false if val.length < minOidLength
	return true

objectTypes = ["any", "blob", "commit", "tag", "tree"]
args.validators.objectType = (val) ->
	return objectTypes.indexOf val > -1

checkOid = (str, allowLookup = true) ->
	throw new TypeError "OID should be a string" if typeof str isnt "string"
	throw new TypeError "Invalid OID" if not oidRegex.test str
	throw new Error "OID is too short" if str.length < Gitteh.minOidLength
	throw new TypeError "Invalid OID" if not allowLookup and str.length isnt 40

wrapCallback = (orig, cb) ->
	return (err) ->
		return orig err if err?
		cb.apply null, Array.prototype.slice.call arguments, 1

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

Gitteh.Index = Index = (nativeIndex) ->
	

Gitteh.Repository = Repository = (nativeRepo) ->
	if nativeRepo not instanceof NativeRepository
		throw new Error "Don't construct me, see gitteh.(open|init)Repository"

	immutable(@, nativeRepo)
		.set("bare")
		.set("path")
		.set("workDir", "workingDirectory")
		.set("references")
	@exists = =>
		[oid, cb] = args
			oid: type: "oid"
			cb: type: "function"
		nativeRepo.exists oid, cb
	@object = =>
		[oid, type, cb] = args
			oid: type: "oid"
			type: type: "objectType", default: "any"
			cb: type: "function"
		nativeRepo.object oid, type, wrapCallback cb, (object) =>
			clazz = switch object._type
				when types.commit then Commit
				when types.tree then Tree
				when types.blob then Blob
				when types.tag then Tag
				else undefined
			return cb new TypeError("Unexpected object type") if clazz is undefined
			return cb null, new clazz @, object
	@blob = (oid, cb) => @object oid, "blob", cb
	@commit = (oid, cb) => @object oid, "commit", cb
	@tag = (oid, cb) => @object oid, "tag", cb
	@tree = (oid, cb) => @object oid, "tree", cb
	@reference = =>
		[name, resolve, cb] = args
			name: type: "string"
			resolve: type: "bool"
			cb: type: "function"
		nativeRepo.reference name, resolve, cb
	@ref = @reference
	return @

Gitteh.openRepository = ->
	[path, cb] = args
		path: type: "string"
		cb: type: "function"
	bindings.openRepository path, wrapCallback cb, (repo) ->
		cb null, new Repository repo

Gitteh.initRepository = () ->
	[path, bare, cb] = args
		path: type: "string"
		bare: type: "bool", default: false
		cb: type: "function"
	bindings.initRepository path, bare, wrapCallback cb, (repo) ->
		cb null, new Repository repo
