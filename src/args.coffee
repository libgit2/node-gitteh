module.exports = fn = (params) ->
	paramList = for own name, param of params
		param.name = name
		param
	ret = []

	numRequired = 0
	leftRequired = for param in paramList
		break if param.hasOwnProperty("default")
		numRequired++
		param
	paramList.splice 0, leftRequired.length
	rightRequired = for param in paramList.slice(0).reverse()
		break if param.hasOwnProperty("default")
		numRequired++
		param
	rightRequired = rightRequired.reverse()
	paramList.splice -rightRequired.length

	args = Array.prototype.slice.call fn.caller.arguments
	throw new Error("Not enough arguments.") if args.length < numRequired

	left = args.splice 0, leftRequired.length
	right = args.splice -rightRequired.length

	argn = 0
	for arg, i in left
		argn++
		param = leftRequired[i]
		if not fn.validators[param.type] arg
			throw new TypeError "#{param.name} (#{argn}) is not a valid #{param.type}"
		ret.push arg

	for param, i in paramList
		argn++
		if args.length > i
			arg = args[i]
			if not fn.validators[param.type] arg
				throw new TypeError "#{param.name} (#{argn}) is not a valid #{param.type}"
			ret.push arg
		else ret.push param.default
	for arg, i in right
		argn++
		param = rightRequired[i]
		if not fn.validators[param.type] arg
			throw new TypeError "#{param.name} (#{argn}) is not a valid #{param.type}"
		ret.push arg
	return ret


fn.oidRegex = oidRegex = /^[a-zA-Z0-9]{0,40}$/
objectTypes = ["any", "blob", "commit", "tag", "tree"]
remoteDirs = ["push", "fetch"]

fn.validators = 
	string: (val) ->
		return typeof val is "string"

	function: (val) -> return typeof val is "function"

	bool: (val) ->
		return typeof val is "boolean"

	oid: (val) ->
		return false if typeof val isnt "string"
		return false if not oidRegex.test val
		return false if val.length < fn.minOidLength
		return true

	objectType: (val) ->
		return objectTypes.indexOf val > -1

	remoteDir: (val) ->
		return remoteDirs.indexOf val > -1

	array: (val) ->
		return val instanceof Array

	object: (val) ->
		return typeof val is "object"