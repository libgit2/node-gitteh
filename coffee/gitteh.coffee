{EventEmitter} = require "events"
async = require "async"
fs = require "fs"
_path = require "path"
args = require "./args"
bindings = require "../build/Debug/gitteh"

{minOidLength, types, NativeRepository, NativeRemote} = bindings
module.exports = Gitteh = {}

getPrivate = (obj) ->
	getPrivate.lock++
	return obj._private
getPrivate.lock = 0

createPrivate = (obj) ->
	_priv = {}
	Object.defineProperty obj, "_private",
		enumerable: false
		configurable: false
		get: ->
			throw new Error "Bad request" if not getPrivate.lock--
			return _priv
	return _priv

wrapCallback = (orig, cb) ->
	return (err) ->
		return orig err if err?
		cb.apply null, Array.prototype.slice.call arguments, 1

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

checkOid = (str, allowLookup = true) ->
	throw new TypeError "OID should be a string" if typeof str isnt "string"
	throw new TypeError "Invalid OID" if not oidRegex.test str
	throw new Error "OID is too short" if str.length < bindings.minOidLength
	throw new TypeError "Invalid OID" if not allowLookup and str.length isnt 40

Gitteh.Signature = Signature = (obj) ->
	immutable(@, obj)
		.set("name")
		.set("email")
		.set("time")
		.set("offset")
	return @

Gitteh.Refspec = Refspec = (src, dst) ->
	_priv = createPrivate @

	_priv.srcRoot = if src? and src[-1..] is "*" then src[0...-1] else src
	_priv.dstRoot = if dst? and dst[-1..] is "*" then dst[0...-1] else dst

	immutable(@, {src, dst})
		.set("src")
		.set("dst")
	return @
Refspec.prototype.matchesSrc = (ref) ->
	_priv = getPriv @
	return false if ref.length <= _priv.srcRoot.length
	return ref.indexOf(_priv.srcRoot) is 0
Refspec.prototype.matchesDst = (ref) ->
	_priv = getPriv @
	return false if ref.length <= _priv.dstRoot.length
	return ref.indexOf(_priv.dstRoot) is 0
Refspec.prototype.transformTo = (ref) ->
	throw new Error "Ref doesn't match src." if not @matchesSrc ref
	return "#{@dst[0...-2]}#{ref[(@src.length-2)..]}"
Refspec.prototype.transformFrom = (ref) ->
	throw new Error "Ref doesn't match dst." if not @matchesDst ref
	return "#{@src[0...-2]}#{ref[(@dst.length-2)..]}"

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
	return @
Commit.prototype.tree = (cb) ->
	@repository.tree @treeId, cb

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
Tag.prototype.target = (cb) ->
	@repository.object @targetId, @type, cb

Gitteh.Remote = Remote = (@repository, nativeRemote) ->
	_priv = createPrivate @
	_priv.native = nativeRemote
	_priv.connected = false

	if nativeRemote not instanceof NativeRemote
		throw new Error "Don't construct me, see Repository.remote()"

	Object.defineProperty @, "connected",
		get: -> return _priv.connected
		enumerable: true
		configurable: false

	immutable(@, nativeRemote)
		.set("name")
		.set("url")

	fetchSpec = new Refspec nativeRemote.fetchSpec.src, nativeRemote.fetchSpec.dst
	pushSpec = new Refspec nativeRemote.pushSpec.src, nativeRemote.pushSpec.dst
	immutable(@, {fetchSpec, pushSpec})
		.set("fetchSpec")
		.set("pushSpec")
	return @

Remote.prototype.connect = ->
	_priv = getPrivate @
	[dir, cb] = args
		dir: type: "remoteDir"
		cb: type: "function"
	dir = if dir is "push" then bindings.GIT_DIR_PUSH else bindings.GIT_DIR_FETCH
	_priv.native.connect dir, wrapCallback cb, (refs) =>
		refNames = Object.keys refs

		# Determine symref for HEAD.
		headOid = refs["HEAD"]
		for ref, oid of refs
			continue if ref is "HEAD"
			if oid is headOid
				headRef = fetchSpec.transformTo ref
				immutable(@, {headRef}).set "headRef", "HEAD"
				break

		immutable(@, {refNames}).set "refNames", "refs"
		_priv.connected = true
		cb()
Remote.prototype.fetch = ->
	_priv = getPrivate @
	throw new Error "Remote isn't connected." if not connected
	[progressCb, cb] = args
		progressCb: type: "function"
		cb: type: "function"

	updateTimer = null
	update = =>
		{bytes, total, done} = _priv.native.stats
		progressCb bytes, total, done
		updateTimer = setTimeout update, 500
	setTimeout update, 500

	_priv.native.download (err) =>
		clearTimeout updateTimer
		return cb err if err?
		_priv.native.updateTips wrapCallback cb, =>
			cb()

Gitteh.Index = Index = (nativeIndex) ->

Gitteh.Repository = Repository = (nativeRepo) ->
	if nativeRepo not instanceof NativeRepository
		throw new Error "Don't construct me, see gitteh.(open|init)Repository"
	_priv = createPrivate @
	_priv.native = nativeRepo

	immutable(@, nativeRepo)
		.set("bare")
		.set("path")
		.set("workDir", "workingDirectory")
		.set("remotes")
		.set("references")
	return @
Repository.prototype.exists = ->
	_priv = getPrivate @
	[oid, cb] = args
		oid: type: "oid"
		cb: type: "function"
	_priv.native.exists oid, cb
Repository.prototype.object = ->
	_priv = getPrivate @
	[oid, type, cb] = args
		oid: type: "oid"
		type: type: "objectType", default: "any"
		cb: type: "function"
	_priv.native.object oid, type, wrapCallback cb, (object) =>
		clazz = switch object._type
			when types.commit then Commit
			when types.tree then Tree
			when types.blob then Blob
			when types.tag then Tag
			else undefined
		return cb new TypeError("Unexpected object type") if clazz is undefined
		return cb null, new clazz @, object
Repository.prototype.blob = (oid, cb) -> @object oid, "blob", cb
Repository.prototype.commit = (oid, cb) -> @object oid, "commit", cb
Repository.prototype.tag = (oid, cb) -> @object oid, "tag", cb
Repository.prototype.tree = (oid, cb) -> @object oid, "tree", cb
Repository.prototype.reference = ->
	_priv = getPrivate @
	[name, resolve, cb] = args
		name: type: "string"
		resolve: type: "bool"
		cb: type: "function"
	nativeRepo.reference name, resolve, cb
Repository.prototype.ref = Repository.prototype.reference
Repository.prototype.remote = ->
	_priv = getPrivate @
	[name, cb] = args
		name: type: "string"
		cb: type: "function"
	_priv.native.remote name, wrapCallback cb, (remote) =>
		return cb null, new Remote @, remote
Repository.prototype.createRemote = ->
	_priv = getPrivate @
	[name, url, cb] = args
		name: type: "string"
		url: type: "string"
		cb: type: "function"
	_priv.native.createRemote name, url, wrapCallback cb, (remote) =>
		return cb null, new Remote @, remote

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
		# Initialize a fresh repo in the path specified.
		(cb) -> Gitteh.initRepository path, false, cb

		# Create the origin remote with provided URL.
		(repo, cb) ->
			repo.createRemote "origin", url, wrapCallback cb, (remote) ->
				cb null, repo, remote

		# Connect to the remote to commence fetch.
		(repo, remote, cb) ->
			remote.connect "fetch", wrapCallback cb, ->
				cb null, repo, remote

		# Perform the actual fetch, sending progress updates as they come in.
		(repo, remote, cb) ->
			emitProgress = (bytes, done, complete) ->
				emitter.emit "status", bytes, done, complete
			remote.fetch emitProgress, wrapCallback cb, ->
				cb null, repo, remote

		# The connect step earlier resolved remote HEAD. Let's fetch that ref.
		(repo, remote, cb) ->
			repo.ref remote.HEAD, true, wrapCallback cb, (ref) ->
				cb null, repo, remote, ref

		# We now have fully resolved OID head ref. Fetch the commit.
		(repo, remote, headRef, cb) ->
			repo.commit headRef.target, wrapCallback cb, (commit) ->
				cb null, repo, remote, commit

		# Now fetch the tree for the commit.
		(repo, remote, headCommit, cb) ->
			headCommit.tree wrapCallback cb, (tree) ->
				cb null, repo, remote, tree

		# Now we can go ahead and checkout this tree into working directory.
		(repo, remote, headTree, cb) ->
			handleEntry = (dest, entry, cb) ->
				if entry.type is "tree"
					subPath = _path.join dest, entry.name
					async.series [
						# TODO: mode?
						(cb) -> fs.mkdir subPath, cb
						(cb) ->
							repo.tree entry.id, wrapCallback cb, (subtree) ->
								checkoutTree subtree, subPath, cb
					], cb
				else if entry.type is "blob"
					repo.blob entry.id, wrapCallback cb, (blob) ->
						file = fs.createWriteStream _path.join(dest, entry.name), 
							mode: entry.attributes
						file.write blob.data
						file.end()
						cb()
				else
					cb()
			checkoutTree = (tree, dest, cb) ->
				async.forEach tree.entries, handleEntry.bind(null, dest), cb
			checkoutTree headTree, repo.workingDirectory, wrapCallback cb, ->
				cb null, repo
	], (err, repo) ->
		return emitter.emit "error", err if err?

		emitter.emit "complete", repo

	return emitter
