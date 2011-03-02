var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);

var createCommitTests = function(commitFixture) {
	var context = {
		topic: function() {
			return repo.getCommit(commitFixture.id);
		},
		
		"exists": function(commit) {
			assert.isNotNull(commit);
		},
		
		"gives us same object if requested again": function(commit) {
			assert.isTrue(commit === repo.getCommit(commitFixture.id), "Commit objects do not match!");
		},
		
		"has correct id": function(commit) {
			assert.equal(commit.id, commitFixture.id);
		},
		
		"commit id is immutable": function(commit) {
			commit.id = "foo";
			assert.equal(commit.id, commitFixture.id);
		},
		
		"has correct message": function(commit) {
			assert.equal(commit.message, commitFixture.message);
		},
		
		"has correct short message": function(commit) {
			assert.equal(commit.shortMessage, commitFixture.shortMessage);
		},
		
		"short message is immutable": function(commit) {
			commit.shortMessage = "blah blah blah";
			assert.equal(commit.shortMessage, commitFixture.shortMessage);
		},

		"has correct author": function(commit) {
			assert.isNotNull(commit.author);
			assert.equal(commit.author.name, commitFixture.authorName);
			assert.equal(commit.author.email, commitFixture.authorEmail);
		},

		"has correct committer": function(commit) {
			assert.isNotNull(commit.committer);
			assert.equal(commit.committer.name, commitFixture.committerName);
			assert.equal(commit.committer.email, commitFixture.committerEmail);
		},

		"has correct time": function(commit) {
			assert.equal(commit.time.getTime(), commitFixture.time.getTime());
		},

		"has correct tree": function(commit) {
			assert.isTrue(!!commit.getTree());
			assert.equal(commit.getTree().id, commitFixture.tree);
		},
		
		"tree is not redundant": function(commit) {
			assert.isTrue(commit.getTree() === commit.getTree());
		}
	};
	
	if(commitFixture.parents) {
		context["has correct number of parents"] = function(commit) {
			assert.equal(commit.parentCount, commitFixture.parents.length);
		};
		
		context["parents are correct"] = function(commit) {
			commitFixture.parents.forEach(function(commitFixtureParent, i) {
				assert.equal(commit.getParent(i).id, commitFixtureParent);
			});
		};
		
		context["parent length is immutable"] = function(commit) {
			commit.parentCount = -1;
			assert.equal(commit.parentCount, commitFixture.parents.length);
		};

		commitFixture.parents.forEach(function(commitFixtureParent, i) {
			context["parent *" + commitFixtureParent + "* is not redundant"] = function(commit) {
				assert.isTrue(commit.getParent(i) === commit.getParent(i));
				assert.isTrue(commit.getParent(i) === repo.getCommit(commitFixtureParent));
			};
		});
		
	}
	
	return context;
};

vows.describe("Commit").addBatch({
	"First commit": createCommitTests(fixtureValues.FIRST_COMMIT),
	"Second commit": createCommitTests(fixtureValues.SECOND_COMMIT),
	"Third commit": createCommitTests(fixtureValues.THIRD_COMMIT),
	"Fourth commit": createCommitTests(fixtureValues.FOURTH_COMMIT),
	"Fifth commit": createCommitTests(fixtureValues.FIFTH_COMMIT)

		/*
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
			},
			
			"entries": {
				"is correct length": function(tree) {
					assert.equal(tree.length, fixtureValues.FIRST_COMMIT_TREE.entries.length);
				},
				
				"- first entry": {
					topic: function(tree) {
						return tree[0];
					},
					
					"has correct name": function(entry) {
						assert.equal(entry.filename, fixtureValues.FIRST_COMMIT_TREE.entries[0].filename);
					},
					
					"has correct attributes": function(entry) {
						assert.equal(entry.attributes, fixtureValues.FIRST_COMMIT_TREE.entries[0].attributes);
					},
				}
			}
		}
	}*/
}).export(module);