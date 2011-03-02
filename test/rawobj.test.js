var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);
vows.describe("RawObj").addBatch({
	"Opening a raw object": {
		topic: function() {
			return repo.getRawObject(fixtureValues.TEST_TAG.id);
		},
		
		"gives us an object": function(obj) {
			assert.isTrue(!!obj);
		},

		"with the correct *type*": function(obj) {
			assert.equal(obj.type, "tag");
		},
		
		"*type* is immutable": function(obj) {
			obj.type = "foo";
			assert.equal(obj.type, "tag");
		},
		
		"*data* is a Buffer": function(obj) {
			assert.instanceOf(obj.data, Buffer);
		},
		
		"*data* is correct length": function(obj) {
			assert.equal(obj.data.length, fixtureValues.TEST_TAG.rawBody.length);
		},
		
		"*data* is correct content": function(obj) {
			assert.equal(obj.data.toString(), fixtureValues.TEST_TAG.rawBody);
		}
	}
}).export(module);
