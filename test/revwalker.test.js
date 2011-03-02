var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	profiler = require("profiler"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);

vows.describe("RevWalker").addBatch({
	"RevWalker in ": function() {
		return repo.createWalker();
	},
	
	"gives us a walker": function() {
	}
}).export(module);
