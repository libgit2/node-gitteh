bindings = require "../build/Release/gitteh"

NativeRepository = bindings.Repository
NativeCommit = bindings.Commit

Commit = (nativeCommit) ->
	if not nativeCommit instanceof NativeCommit
		throw new Error("Use repository.getCommit/createCommit")

Repository = (nativeRepo) ->
	if not nativeRepo instanceof NativeRepository
		throw new Error("Don't use this directly, see Gitteh.openRepository/Gitteh.initRepository")

	@exists = (oid, cb) ->
		return nativeRepo.exists oid, cb

	@getCommit = (oid, cb) ->
		oid = oid.toString()
		throw new TypeError "Invalid object id." if not oid 
		if cb? then wrappedCb = (err, commit) ->
			return cb err if err?
			return cb null, new Commit commit
		res = nativeRepo.getCommit oid, wrappedCb
		return new Commit res if res instanceof NativeCommit

	Object.defineProperty @, "path",
		value: nativeRepo.path
		writable: false
		enumerable: true

	Object.defineProperty @, "bare",
		value: nativeRepo.bare
		writable: false
		enumerable: true

	return @

Gitteh = 
	openRepository: (path, cb) ->
		if cb? then wrappedCb = (err, repo) ->
			return cb err if err
			return cb null, new Repository repo
		res = bindings.openRepository path, wrappedCb
		return new Repository res if res instanceof NativeRepository
	initRepository: (path, bare, cb) ->
		if cb? then wrappedCb = (err, repo) ->
			return cb err if err
			return cb null, new Repository repo
		res = bindings.initRepository path, bare, wrappedCb
		return new Repository res if res instanceof NativeRepository

module.exports = Gitteh
module.exports.Repository = Repository