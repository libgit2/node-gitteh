should = require "should"

exports.checkImmutable = (o, prop) ->
	val = o[prop]
	o[prop] = "foo"
	o[prop].should.equal val
	delete o[prop]
	should.exist o[prop]
	o[prop].should.equal val
