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
		}
	}
}).export(module);
