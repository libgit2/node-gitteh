var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

vows.describe("Repository").addBatch({
	"Opening an existing bare repository": {
		topic: function() {
			var repo = new gitteh.Repository(fixtureValues.REPO_PATH);
			this.context.repo = repo;
			
			return repo;
		},
		
		"opens correctly": function(repo) {
			assert.instanceOf(repo, gitteh.Repository);
		},
		
		"repo has correct path": function(repo) {
			assert.equal(repo.path, fixtureValues.REPO_PATH);
		},
		
		"repo path is immutable": function(repo) {
			repo.path = "foo";
			assert.equal(repo.path, fixtureValues.REPO_PATH);
		},
		
		"Commits are not redundant": function(repo) {
			assert.isTrue(repo.getCommit(fixtureValues.FIRST_COMMIT.id) === repo.getCommit(fixtureValues.FIRST_COMMIT.id));
		}
	}
}).export(module);
