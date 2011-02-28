var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	profiler = require("profiler"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(path.join(__dirname, "fixtures/gitrepo"));
vows.describe("Commit").addBatch({
	"First commit": {
		topic: function() {
			return repo.getCommit(fixtureValues.FIRST_COMMIT.id);
		},
		
		"exists": function(commit) {
			assert.isNotNull(commit);
		},
		
		"gives us same object if requested again": function(commit) {
			assert.isTrue(commit === repo.getCommit(fixtureValues.FIRST_COMMIT.id), "Commit objects do not match!");
		},
		
		"has correct id": function(commit) {
			assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
		},
		
		"commit id is immutable": function(commit) {
			commit.id = "foo";
			assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
		},
		
		"has correct message": function(commit) {
			assert.equal(commit.message, fixtureValues.FIRST_COMMIT.message);
		},
		
		"has correct short message": function(commit) {
			assert.equal(commit.shortMessage, fixtureValues.FIRST_COMMIT.shortMessage);
		},

		"has correct author": function(commit) {
			assert.isNotNull(commit.author);
			assert.equal(commit.author.name, fixtureValues.FIRST_COMMIT.authorName);
			assert.equal(commit.author.email, fixtureValues.FIRST_COMMIT.authorEmail);
		},
		
		"has correct committer": function(commit) {
			assert.isNotNull(commit.committer);
			assert.equal(commit.committer.name, fixtureValues.FIRST_COMMIT.committerName);
			assert.equal(commit.committer.email, fixtureValues.FIRST_COMMIT.committerEmail);
		},
		
		"has correct time": function(commit) {
			assert.equal(commit.time.getTime(), fixtureValues.FIRST_COMMIT.time.getTime());
		},
		
		"tree": {
			topic: function(commit) {
				this.context.commit = commit;
				return commit.tree;
			},
			
			"is correct tree": function(tree) {
				assert.equal(tree.id, fixtureValues.FIRST_COMMIT_TREE.id);
			},
			
			"gives us same object if requested again": function(wtf, tree) {
				assert.isTrue(tree === this.context.commit.tree);
			}
		}
	}
}).export(module);