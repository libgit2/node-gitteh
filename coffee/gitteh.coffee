{EventEmitter} = require "events"
async = require "async"
args = require "./args"
bindings = require "../build/Debug/gitteh"

{minOidLength, types, NativeRepository, NativeRemote} = bindings
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

remoteDirs = ["push", "fetch"]
args.validators.remoteDir = (val) ->
	return remoteDirs.indexOf val > -1

immutable = (obj, src) ->
	return o = {
		set: (name, target = name) ->
			if Array.isArray src[name]
				Object.defineProperty obj, target,
					get: () -> src[name].slice(0)
					configurable: false
					enumerable: true
				return o
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

Refspec = (obj) ->
	immutable(@, obj)
		.set("src")
		.set("dst")
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

Gitteh.Remote = Remote = (@repository, nativeRemote) ->
	if nativeRemote not instanceof NativeRemote
		throw new Error "Don't construct me, see Repository.remote()"
	nativeRemote.fetchSpec = new Refspec nativeRemote.fetchSpec
	nativeRemote.pushSpec = new Refspec nativeRemote.pushSpec
	connected = false

	Object.defineProperty @, "connected",
		get: -> return connected
		enumerable: true
		configurable: false

	immutable(@, nativeRemote)
		.set("name")
		.set("url")
		.set("fetchSpec")
		.set("pushSpec")
	@connect = =>
		[dir, cb] = args
			dir: type: "remoteDir"
			cb: type: "function"
		dir = if dir is "push" then Gitteh.GIT_DIR_PUSH else Gitteh.GIT_DIR_FETCH
		nativeRemote.connect dir, wrapCallback cb, (refs) =>
			immutable(@, {refs}).set("refs")
			connected = true
			cb()
	@fetch = =>
		throw new Error "Remote isn't connected." if not connected

		updateTimer = null
		update = =>
			{bytes, total, done} = nativeRemote.stats
			@emit "progress", bytes, total, done
			updateTimer = setTimeout update, 500
		setTimeout update, 500

		nativeRemote.download (err) =>
			clearTimeout updateTimer
			return @emit "error", err if err?
			nativeRemote.updateTips =>
				return @emit "error", err if err?
				@emit "complete"

	return @

Remote.prototype = EventEmitter.prototype

wrapCallback = (orig, cb) ->
	return (err) ->
		return orig err if err?
		cb.apply null, Array.prototype.slice.call arguments, 1

module.exports.Repository = Repository = (nativeRepo) ->
	if nativeRepo not instanceof NativeRepository
		throw new Error "Don't construct me, see gitteh.(open|init)Repository"

	immutable(@, nativeRepo)
		.set("bare")
		.set("path")
		.set("workDir", "workingDirectory")
		.set("remotes")
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
	@remote = =>
		[name, cb] = args
			name: type: "string"
			cb: type: "function"
		nativeRepo.remote name, wrapCallback cb, (remote) =>
			return cb null, new Remote @, remote
	@createRemote = =>
		[name, url, cb] = args
			name: type: "string"
			url: type: "string"
			cb: type: "function"
		nativeRepo.createRemote name, url, wrapCallback cb, (remote) =>
			return cb null, new Remote @, remote
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

Gitteh.clone = =>
	[url, path, cb] = args
		url: type: "string"
		path: type: "string"

	emitter = new EventEmitter

	async.waterfall [
		(cb) -> Gitteh.initRepository path, false, cb
		(repo, cb) ->
			repo.createRemote "origin", url, wrapCallback cb, (remote) ->
				cb null, repo, remote
		(repo, remote, cb) ->
			remote.connect "fetch", wrapCallback cb, ->
				cb null, repo, remote
		# TODO: checkout HEAD into working dir.
	], (err, repo, remote) ->
		remote.fetch wrapCallback cb, ->
				cb null, repo, remote
		errorHandler = (err) ->
			emitter.emit "error", err
		progressHandler = (bytes, done, complete) ->
			emitter.emit "status", bytes, done, complete
		completeHandler = () ->
			remote.removeListener "error", errorHandler
			remote.removeListener "progress", progressHandler
			remote.removeListener "complete", completeHandler
			emitter.emit "complete", repo
		remote.on "error", errorHandler
		remote.on "progress", progressHandler
		remote.on "complete", completeHandler

	return emitter
