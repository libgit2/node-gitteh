###
Omg gitteh is freakin' sweet! :)
###

{EventEmitter} = require "events"
async = require "async"
fs = require "fs"
_path = require "path"
args = require "./args"

# env = if process.env.npm_lifecycle_event is "test" then "Debug" else "Release"
env = "Release"
bindings = require "../build/#{env}/gitteh"

(require "segfault-handler").registerHandler() if env is "Debug"

{minOidLength, types, NativeRepository, NativeRemote} = bindings
args.minOidLength = minOidLength

Gitteh = module.exports = {}

_getPrivate = (obj) ->
	_getPrivate.lock++
	return obj._private
_getPrivate.lock = 0

_createPrivate = (obj) ->
	_priv = {}
	Object.defineProperty obj, "_private",
		enumerable: false
		configurable: false
		get: ->
			throw new Error "Bad request" if not _getPrivate.lock--
			return _priv
	return _priv

_wrapCallback = (orig, cb) ->
	return (err) ->
		return orig err if err?
		cb.apply null, Array.prototype.slice.call arguments, 1

_immutable = (obj, src) ->
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


Gitteh.Signature = class Signature
	###
	Contains the name/email/time for a :class:`gitteh::Commit` author/committer
	or :class:`gitteh::Tag` tagger.

	Signatures contain the following *immutable* properties:

	- **name**: *(String)*
	- **email**: *(String)*
	- **time**: *(Date)*
	- **offset**: *(Number)* timezone offset in seconds from GMT.
	###

	constructor: (obj) ->
		_immutable(@, obj)
			.set("name")
			.set("email")
			.set("time")
			.set("offset")


Gitteh.Refspec = class Refspec
	###
	Describes the way remote repository references will be mapped to the local
	repository. 

	For more information refer to http://git-scm.com/book/ch9-5.html
	###

	constructor: (src, dst) ->
		_priv = _createPrivate @

		_priv.srcRoot = if src? and src[-1..] is "*" then src[0...-1] else src
		_priv.dstRoot = if dst? and dst[-1..] is "*" then dst[0...-1] else dst

		_immutable(@, {src, dst})
			.set("src")
			.set("dst")

	matchesSrc: (refName) ->
		###
		Returns true/false if given `refName` matches source of this Refspec.
		###
		_priv = _getPrivate @
		return false if refName.length <= _priv.srcRoot.length
		return refName.indexOf(_priv.srcRoot) is 0

	matchesDst: (refName) ->
		###
		Returns true/false if given `refName` matches destination of this
		Refspec.
		###
		_priv = _getPrivate @
		return false if refName.length <= _priv.dstRoot.length
		return refName.indexOf(_priv.dstRoot) is 0

	transformTo: (refName) ->
		###
		Transforms given `refName` to destination, provided it matches src
		pattern, and throws an error if it doesn't.
		###
		throw new Error "Ref doesn't match src." if not @matchesSrc refName
		return "#{@dst[0...-2]}#{refName[(@src.length-2)..]}"

	transformFrom: (refName) ->
		###
		Transforms provided refName from destination back to source, provided it
		matches dst pattern, and throws an Error if it doesn't. This is the
		reverse of :func:`gitteh::Refspec.transformTo`
		###
		throw new Error "Ref doesn't match dst." if not @matchesDst refName
		return "#{@src[0...-2]}#{refName[(@dst.length-2)..]}"


Gitteh.Commit = class Commit
	###
	Commits, made by an author, and an optional different committer, contain a
	message, an associated :class:`gitteh::Tree`, and zero or more parent
	:class:`gitteh::Commit` objects. Zero parents generally indicate the initial 
	commit for the repository. More than one parent commits indicate a merge
	commit.

	Properties:

	* **id**: *(String)* OID of this commit (SHA1 hash)
	* **treeId**: *(String)* OID of associated :class:`gitteh::Tree`
	* **parents**: *(String[]) list of parent commit OIDs
	* **message**: *(String)*
	* **messageEncoding**: *(???)* ??? TODO:
	* **author**: (:class:`gitteh::Signature`)
	* **committer**: (:class:`gitteh::Signature`)
	###

	constructor: (@repository, obj) ->
		obj.author = new Signature obj.author
		obj.committer = new Signature obj.committer
		_immutable(@, obj)
			.set("id")
			.set("tree", "treeId")
			.set("parents")
			.set("message")
			.set("messageEncoding")
			.set("author")
			.set("committer")

	tree: (cb) ->
		###
		Fetches the :class:`gitteh::Tree` for this Commit. Shortcut for calling
		:func:`gitteh::Repository.tree` with this commits `treeId`.
		###
		@repository.tree @treeId, cb	


Gitteh.Tree = class Tree
	###
	A Tree contains a list of named entries, which can either be
	:class:`gitteh::Blob` objects or nested :class:`gitteh::Tree` objects. Each
	entry is referenced by its OID.

	Properties:

	* **id**: *(String)* OID of this Tree.
	* **entries**: *(TreeEntry[])* 

	## Tree Entries
	
	Each element of a Tree contains the following properties:

	* **id**: *(String)* OID this entry points to.
	* **name**: *(String)* file name of this entry.
	* **type**: *(String)* kind of object pointed to by this entry
	* **filemode**: *(Number)* UNIX file filemode for this entry.
	###

	constructor: (@repository, obj) ->
		obj._entries = obj.entries
		obj.entries = []
		for origEntry in obj._entries
			obj.entries.push entry = {}
			_immutable(entry, origEntry)
				.set("id")
				.set("name")
				.set("type")
				.set("filemode")
		_immutable(@, obj)
			.set("id")
			.set("entries")


Gitteh.Blob = class Blob
	###
	Contains raw data for a file stored in Git.

	Properties:

	* **id**: *(String)* OID of this Blob.
	* **data**: *(Buffer)* a standard Node buffer containing binary data.
	###

	constructor: (@repository, obj) ->
		_immutable(@, obj)
			.set("id")
			.set("data")


Gitteh.Tag = class Tag
	###
	Git tags are similar to references, and indeed "lightweight" Git tags are 
	actually implemented as :class:`gitteh::Reference` with a name prefix
	of "tags/". When additional metadata is needed (message/name/email/GPG
	signature), a proper heavyweight Tag object is used.

	Properties:

	* **id**: *(String)* OID of this Tag.
	* **name**: *(String)*
	* **message**: *(String)*
	* **tagger**: *(Signature)*
	* **targetId**: *(String)* OID this Tag points to
	* **type**: *(String)* the type of object this Tag points to.
	###

	constructor: (@repository, obj) ->
		obj.tagger = new Signature obj.tagger
		_immutable(@, obj)
			.set("id")
			.set("name")
			.set("message")
			.set("tagger")
			.set("target", "targetId")
			.set("type")

	target: (cb) ->
		###
		Convenience method to get the object this Tag points to. Shorthand for
		calling :func:`gitteh::Repository.object` with this *targetId*
		###
		@repository.object @targetId, @type, cb


Gitteh.Remote = class Remote
	###
	Remotes designate the location and rules of remote Git repositories. Remotes
	can be obtained by using :func:`gitteh::Repository.remote`

	Properties:

	* **connected**: *(Boolean)* true if there is an active connection to the Remotes' endpoint.
	* **name**: *(String)*
	* **url**: *(String)* address of Remotes' endpoint
	* **fetchSpec**: (:class:`gitteh::Refspec`) Refspec used when fetching from Remote
	* **pushSpec**: (:class:`gitteh::Refspec`) Refspec used when pushing to Remote
	* **HEAD**: *(String)* the remote HEAD reference name (only set after connected to Remote)
	* **refs**: *(String[])* names of references on remote (only set after connected to Remote)
	###

	constructor: (@repository, nativeRemote) ->
		_priv = _createPrivate @
		_priv.native = nativeRemote
		_priv.connected = false

		if nativeRemote not instanceof NativeRemote
			throw new Error "Don't construct me, see Repository.remote()"

		Object.defineProperty @, "connected",
			get: -> return _priv.connected
			enumerable: true
			configurable: false

		_immutable(@, nativeRemote)
			.set("name")
			.set("url")

		fetchSpec = new Refspec nativeRemote.fetchSpec.src, nativeRemote.fetchSpec.dst
		pushSpec = new Refspec nativeRemote.pushSpec.src, nativeRemote.pushSpec.dst
		_immutable(@, {fetchSpec, pushSpec})
			.set("fetchSpec")
			.set("pushSpec")

	connect: ->
		###
		Opens a connection to the :class:`gitteh::Remote` endpoint. This is
		needed before :func:`gitteh::Remote.fetch` or :func:`gitteh::Remote.push`
		can be called. `direction` must be supplied as either "push" or "fetch"
		and `cb` will be called once Remote is connected.
		###
		_priv = _getPrivate @
		[dir, cb] = args
			dir: type: "remoteDir"
			cb: type: "function"
		dir = if dir is "push" then bindings.GIT_DIRECTION_PUSH else bindings.GIT_DIRECTION_FETCH
		_priv.native.connect dir, _wrapCallback cb, (refs) =>
			refNames = Object.keys refs

			# Determine symref for HEAD.
			headOid = refs["HEAD"]
			for ref, oid of refs
				continue if ref is "HEAD"
				if oid is headOid
					headRef = @fetchSpec.transformTo ref
					_immutable(@, {headRef}).set "headRef", "HEAD"
					break

			_immutable(@, {refNames}).set "refNames", "refs"
			_priv.connected = true
			cb()

	fetch: ->
		###
		Fetches Git objects from remote that do not exist locally. `progressCb`
		will be called regularly to notify callers of fetch progress and `cb`
		will be called once fetch has completed.
		###
		_priv = _getPrivate @
		throw new Error "Remote isn't connected." if not @connected
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
			_priv.native.updateTips _wrapCallback cb, =>
				cb()


Gitteh.Index = class Index
	###
	The Git index is used to stage changed files before they are written to the 
	repository proper. Bindings for the Index are currently minimal, expect
	this to change in a newer version.
	###

	constructor: (nativeIndex) ->
		_priv = _createPrivate @
		_priv.native = nativeIndex

	readTree: ->
		###
		Updates the Git index to reflect the state of provided
		:class:`gitteh::Tree` (using OID passed from `id` parameter). `cb` will
		be called once index update has completed.
		###
		_priv = _getPrivate @
		[id, cb] = args
			id: type: "oid"
			cb: type: "function"
		_priv.native.readTree id, cb

	write = ->
		###
		Synchronizes the in-memory Git index with the indexfile located in
		repository, and calls `cb` once synchronization is complete.
		###
		_priv = _getPrivate @
		[cb] = args
			cb: type: "function"
		_priv.native.write cb


Gitteh.Reference = class Reference
	###
	A Reference is a named pointer to a :class:`gitteh::Commit` object. That is,
	refs are the DNS of Git-land. References can either be direct or symbolic.
	Direct references point to the object id of a commit. Symbolic refs point
	to other references.

	References can be obtained using :func:`gitteh::Repository.reference` and
	created using :func:`gitteh::Repository.createReference`.

	Properties:

	* **name**: *(String)*
	* **direct**: *(Boolean)* true if Reference points directly to an object id.
	* **packed**: *(Boolean)* true if Reference is in a packfile
	* **target**: *(String)* object id reference points to, or another reference
	name if not a direct reference.
	* **repository**: (:class:`gitteh::Repository`) owner of this Reference.
	###

	constructor: (repo, nativeRef) ->
		_priv = _createPrivate @
		_priv.native = nativeRef
		_immutable(@, nativeRef)
			.set("name")
			.set("direct")
			.set("packed")
			.set("target")
		_immutable(@, {repo}).set "repo", "repository"


Gitteh.Repository = class Repository
	###
	Represents a local Git repository that has been opened by Gitteh. Objects
	such as :class:`gitteh::Commit`, :class:`gitteh.Tree` and 
	:class:`gitteh.Reference` can be obtained and created from a Repository.
	
	Repositories can be *bare* - they will not have a working directory, in this
	case the contents of what is usually in a .git subdirectory will be in the
	top level.

	Properties:

	* **bare**: *(Boolean)* true if this repository is bare.
	* **path**: *(String)* location of the Git metadata directory
	* **workingDirectory**: *(String)* location of the working directory, if applicable (non-bare repository)
	* **remotes**: *(String[])*  names of remotes configured for this repository
	* **references**: *(String[])* names of references contained in this repository.
	* **index**: (:class:`gitteh::Index`) The Git index for this repository.
	###

	constructor: (nativeRepo) ->
		if nativeRepo not instanceof NativeRepository
			throw new Error "Don't construct me, see gitteh.(open|init)Repository"
		_priv = _createPrivate @
		_priv.native = nativeRepo

		_immutable(@, nativeRepo)
			.set("bare")
			.set("path")
			.set("workDir", "workingDirectory")
			.set("remotes")
			.set("references")
			.set("submodules")
		index = new Index nativeRepo.index
		_immutable(@, {index}).set "index"

	exists: (oid, cb) ->
		###
		Checks if an object with given `oid` exists. Calls `cb` with result.
		###
		_priv = _getPrivate @
		[oid, cb] = args
			oid: type: "oid"
			cb: type: "function"
		_priv.native.exists oid, cb

	object: ->
		###
		Fetches an object with given `oid` and returns the result to provided 
		`cb`. The object returned will be a Gitteh wrapper corresponding to the
		type of Git object fetched.

		Alternatively, objects with an expected type can be fetched using the
		:func:`gitteh::Repository.blob` :func:`gitteh::Repository.commit`
		:func:`gitteh::Repository.tag` :func:`gitteh::Repository.tree` and
		:func:`gitteh::Repository.reference` methods.
		###
		_priv = _getPrivate @
		[oid, type, cb] = args
			oid: type: "oid"
			type: type: "objectType", default: "any"
			cb: type: "function"
		_priv.native.object oid, type, _wrapCallback cb, (object) =>
			clazz = switch object._type
				when types.commit then Commit
				when types.tree then Tree
				when types.blob then Blob
				when types.tag then Tag
				else undefined
			return cb new TypeError("Unexpected object type") if clazz is undefined
			return cb null, new clazz @, object

	blob: (oid, cb) ->
		###
		Fetches a :class:`gitteh::Blob` object with given `oid` from the
		repository. This is a stricter variant of :func:`gitteh::Repository.object`
		- an error will be thrown if requested object isnt a Blob.
		###
		@object oid, "blob", cb

	commit: (oid, cb) ->
		###
		Fetches a :class:`gitteh::Commit` object with given `oid` from the
		repository. This is a stricter variant of :func:`gitteh::Repository.object`
		- an error will be thrown if requested object isnt a Commit.
		###
		@object oid, "commit", cb

	tag: (oid, cb) ->
		###
		Fetches a :class:`gitteh::Tag` object with given `oid` from the
		repository. This is a stricter variant of :func:`gitteh::Repository.object`
		- an error will be thrown if requested object isnt a Tag.
		###
		@object oid, "tag", cb

	tree: (oid, cb) ->
		###
		Fetches a :class:`gitteh::Tree` object with given `oid` from the
		repository. This is a stricter variant of :func:`gitteh::Repository.object`
		- an error will be thrown if requested object isnt a Tree.
		###
		@object oid, "tree", cb

	reference: ->
		###
		Fetches a :class:`gitteh::Reference` object with given `oid` from the
		repository. This is a stricter variant of :func:`gitteh::Repository.object`
		- an error will be thrown if requested object isnt a Reference.
		###
		_priv = _getPrivate @
		[name, resolve, cb] = args
			name: type: "string"
			resolve: type: "bool", default: false
			cb: type: "function"
		_priv.native.reference name, resolve, _wrapCallback cb, (ref) =>
			cb null, new Reference @, ref

	createReference: ->
		###
		Creates a new reference, which can either by direct or symbolic.

		Parameters:

		* **name**: *(String)*
		* **target**: *(String)* reference/oid targetted by the new reference.
		* **[force=false]**: *(String)* force creation of this reference, destroying the reference with same name, if it exists.
		* **cb**: *(String)* called when reference has been created.
		###
		_priv = _getPrivate @
		[name, target, force, cb] = args
			name: type: "string"
			target: type: "string"
			force: type: "bool", default: false
			cb: type: "function"
		fn = "createSymReference"
		if target.length is 40 and args.oidRegex.test target
			fn = "createOidReference"
		_priv.native[fn] name, target, force, _wrapCallback cb, (ref) =>
			cb null, new Reference @, ref

	remote: ->
		###*
		Loads a remote with given `name`.
		###
		_priv = _getPrivate @
		[name, cb] = args
			name: type: "string"
			cb: type: "function"
		_priv.native.remote name, _wrapCallback cb, (remote) =>
			return cb null, new Remote @, remote

	createRemote: ->
		###*
		Create a new :class:`gitteh::Remote` for this repository with the given
		`name` and `url`. Calls `cb` when the operation has completed.
		###
		_priv = _getPrivate @
		[name, url, cb] = args
			name: type: "string"
			url: type: "string"
			cb: type: "function"
		_priv.native.createRemote name, url, _wrapCallback cb, (remote) =>
			return cb null, new Remote @, remote

	createBlobFromDisk: ->
		###
		Creates a new Blob for this repository from the file-content loaded
		from `path`. Calls `cb` when the operation has completed.
		###
		_priv = _getPrivate @
		[path, cb] = args
			path: type: "string"
			cb: type: "function"
		_priv.native.createBlobFromDisk path, _wrapCallback cb, (blob) =>
			return cb null, blob

	createBlobFromBuffer: ->
		###
		Creates a new Blob for this repository from the data supplied
		as `buffer`. Calls `cb` when the operation has completed.
		###
		_priv = _getPrivate @
		[buffer, cb] = args
			buffer: type: "object"
			cb: type: "function"
		_priv.native.createBlobFromBuffer buffer, _wrapCallback cb, (blob) =>
			return cb null, blob

	createTree: ->
		###
		Creates a new Tree for this repository from the `entities`
		that contain the content (as a blob), the name and the filemode
		of the tree-entities that will be in the new Tree.
		Calls `cb` when the operation has completed.
		###
		_priv = _getPrivate @
		[entities, cb] = args
			entities: type: "array"
			cb: type: "function"
		_priv.native.createTree entities, _wrapCallback cb, (tree) =>
			return cb null, tree

	createCommit: ->
		###
		Creates a new Commit for this repository from `data`.
		`data` should contain the following keys:

		* **updateref**: updates this reference to the new commit. (optional, default: do not update any refs)
		* **author**: a signature of the author (optional, default: committer is used)
		* **committer**: a signature of the committer
		* **message**: the message of the commit
		* **tree**: the id of a tree object
		* **parents**: an array of parents.

		A signature has the following keys:

		* **name**: the name of the author/committer
		* **email**: the email of the author/committer
		* **time**: the time of the commit (optional)
		* **offset**: timezone offset of the commit-time (optional)
		###
		_priv = _getPrivate @
		[data, cb] = args
			data: type: "object"
			cb: type: "function"
		_priv.native.createCommit data, _wrapCallback cb, (commit) =>
			return cb null, commit

###*
 * Alias of {@link #reference}.
 * @param {String} oid id of reference to be fetched.
 * @param {Function} cb called when reference has been fetched.
 * @see #reference
###
Repository.prototype.ref = Repository.prototype.reference


Gitteh.openRepository = ->
	###
	Opens a local Git repository with the given `path`. Calls `cb` once
	a :class:`gitteh::Repository` has been opened.
	###
	[path, cb] = args
		path: type: "string"
		cb: type: "function"
	bindings.openRepository path, _wrapCallback cb, (repo) ->
		cb null, new Repository repo

Gitteh.initRepository = () ->
	###
	Creates a new Git repository at `path`, and calls `cb` providing the new 
	:class:`gitteh::Repository` when operation has completed.

	If `bare` is specified as true the repository will be created without a
	working directory. For more information see (TODO: link to page describing
	bare repositories.)
	###
	[path, bare, cb] = args
		path: type: "string"
		bare: type: "bool", default: false
		cb: type: "function"
	bindings.initRepository path, bare, _wrapCallback cb, (repo) ->
		cb null, new Repository repo

Gitteh.clone = =>
	###
	Provides high level "git clone" functionality.

	Creates a new repository on the local filesystem at `path`, creates a 
	default "origin" :class:`gitteh::Remote` with the provided `url`, fetches
	the references from the remote, checks out the master branch into the
	working directory.

	Currently, libgit2 only supports HTTP/Git protocols - there is no support
	for git+ssh yet.
	###
	[url, path, cb] = args
		url: type: "string"
		path: type: "string"

	emitter = new EventEmitter

	async.waterfall [
		# Initialize a fresh repo in the path specified.
		(cb) -> Gitteh.initRepository path, false, cb

		# Create the origin remote with provided URL.
		(repo, cb) ->
			repo.createRemote "origin", url, _wrapCallback cb, (remote) ->
				cb null, repo, remote

		# Connect to the remote to commence fetch.
		(repo, remote, cb) ->
			remote.connect "fetch", _wrapCallback cb, ->
				cb null, repo, remote

		# Perform the actual fetch, sending progress updates as they come in.
		(repo, remote, cb) ->
			emitProgress = (bytes, done, complete) ->
				emitter.emit "status", bytes, done, complete
			remote.fetch emitProgress, _wrapCallback cb, ->
				cb null, repo, remote

		# The connect step earlier resolved remote HEAD. Let's fetch that ref.
		(repo, remote, cb) ->
			repo.ref remote.HEAD, true, _wrapCallback cb, (ref) ->
				cb null, repo, remote, ref

		# We now have fully resolved OID head ref. Create a local branch.
		(repo, remote, headRef, cb) ->
			refName = remote.fetchSpec.transformFrom remote.HEAD
			repo.createReference refName, headRef.target, _wrapCallback cb, ->
				cb null, repo, remote, headRef

		# And now fetch the commit.
		(repo, remote, headRef, cb) ->
			repo.commit headRef.target, _wrapCallback cb, (commit) ->
				cb null, repo, remote, commit

		# Now fetch the tree for the commit.
		(repo, remote, headCommit, cb) ->
			headCommit.tree _wrapCallback cb, (tree) ->
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
							repo.tree entry.id, _wrapCallback cb, (subtree) ->
								checkoutTree subtree, subPath, cb
					], cb
				else if entry.type is "blob"
					repo.blob entry.id, _wrapCallback cb, (blob) ->
						file = fs.createWriteStream _path.join(dest, entry.name), 
							mode: entry.filemode
						file.write blob.data
						file.end()
						cb()
				else
					cb()
			checkoutTree = (tree, dest, cb) ->
				async.forEach tree.entries, handleEntry.bind(null, dest), cb
			checkoutTree headTree, repo.workingDirectory, _wrapCallback cb, ->
				cb null, repo, remote, headTree

		# Update the git index with the tree we just checked out.
		(repo, remote, headTree, cb) ->
			repo.index.readTree headTree.id, _wrapCallback cb, ->
				cb null, repo, remote

		# Now write the index back to disk.
		(repo, remote, cb) ->
			repo.index.write _wrapCallback cb, ->
				cb null, repo, remote
	], (err, repo) ->
		return emitter.emit "error", err if err?

		emitter.emit "complete", repo

	return emitter
