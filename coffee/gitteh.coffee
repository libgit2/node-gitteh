bindings = require "../build/Release/gitteh"

NativeRepository = bindings.Repository

Repository = (nativeRepo) ->
	if not nativeRepo instanceof NativeRepository
		throw new Error("Don't use this directly, see Gitteh.openRepository/Gitteh.initRepository")

	@exists = (oid, cb) ->
		return nativeRepo.exists oid, cb

	Object.defineProperty @, "path",
		value: nativeRepo.path
		writable: false
		enumerable: true

	return @

Gitteh = 
	openRepository: (path, cb) ->
		if cb? then wrappedCb = (err, repo) ->
			return err if err
			return new Repository repo
		res = bindings.openRepository path, wrappedCb
		console.log res
		console.log res instanceof NativeRepository
		return new Repository res if res instanceof NativeRepository

module.exports = Gitteh
module.exports.Repository = Repository