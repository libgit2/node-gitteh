/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
 var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = gitteh.openRepository(fixtureValues.REPO_PATH);

var createCommitTests = function(topic, commitFixture) {
	var context = {
		topic: topic,
		
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
		
		"commit id cannot be deleted": function(commit) {
			delete commit.id;
			assert.equal(commit.id, commitFixture.id);
		},
		
		"has correct message": function(commit) {
			assert.equal(commit.message, commitFixture.message);
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
			assert.equal(commit.committer.time.getTime(), commitFixture.time.getTime());
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
		
		context["parent length cannot be deleted"] = function(commit) {
			delete commit.parentCount;
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

var createAsyncCommitTests = function(commitFixture) {
	return createCommitTests(function() {
		repo.getCommit(commitFixture.id, this.callback);
	}, commitFixture);
};
var createSyncCommitTests = function(commitFixture) {
	return createCommitTests(function() {
		return repo.getCommit(commitFixture.id);
	}, commitFixture);
};

vows.describe("Commit").addBatch({
	"First commit (async)": createAsyncCommitTests(fixtureValues.FIRST_COMMIT),
	"Second commit (async)": createAsyncCommitTests(fixtureValues.SECOND_COMMIT),
	"Third commit (async)": createAsyncCommitTests(fixtureValues.THIRD_COMMIT),
	"Fourth commit (async)": createAsyncCommitTests(fixtureValues.FOURTH_COMMIT),
	"Fifth commit (async)": createAsyncCommitTests(fixtureValues.FIFTH_COMMIT),
	
	"First commit (sync)": createSyncCommitTests(fixtureValues.FIRST_COMMIT),
	"Second commit (sync)": createSyncCommitTests(fixtureValues.SECOND_COMMIT),
	"Third commit (sync)": createSyncCommitTests(fixtureValues.THIRD_COMMIT),
	"Fourth commit (sync)": createSyncCommitTests(fixtureValues.FOURTH_COMMIT),
	"Fifth commit (sync)": createSyncCommitTests(fixtureValues.FIFTH_COMMIT),
	
	"Creating a new commit *synchronously*": {
		topic: function() {
			return repo.createCommit();
		},
		
		"gives us an identity Commit": function(commit) {
			assert.isTrue(!!commit);
			assert.isNull(commit.id);
			assert.isNull(commit.message);
			assert.isNull(commit.author);
			assert.isNull(commit.committer);
			assert.throws(function() {
				commit.getTree();
			}, Error);
		},
	},
	
	"Creating a new commit *asynchronously*": {
		topic: function() {
			repo.createCommit(this.callback);
		},
		
		"gives us an identity Commit": function(commit) {
			assert.isTrue(!!commit);
			assert.isNull(commit.id);
			assert.isNull(commit.message);
			assert.isNull(commit.author);
			assert.isNull(commit.committer);
			assert.throws(function() {
				commit.getTree();
			}, Error);
		}
	},
	
	"Getting commit tree *synchronously*": {
		topic: function() {
			var commit = repo.getCommit(fixtureValues.FIRST_COMMIT.id);
			return commit.getTree();
		},
		
		"returns correct tree": function(tree) {
			assert.isTrue(!!tree);
			assert.equal(tree.id, fixtureValues.FIRST_TREE.id);
		}
	},
	
	"Getting commit tree *asynchronously*": {
		topic: function() {
			var commit = repo.getCommit(fixtureValues.FIRST_COMMIT.id);
			commit.getTree(this.callback);
		},
		
		"returns correct tree": function(tree) {
			assert.isTrue(!!tree);
			assert.equal(tree.id, fixtureValues.FIRST_TREE.id);
		}
	},
	
	"Setting commit tree *asynchronously*": {
		topic: function() {
			var commit = this.context.commit = repo.createCommit();
			var tree = this.context.tree = repo.getTree(fixtureValues.FIRST_TREE.id);
			commit.setTree(tree, this.callback);
		},

		"sets tree correctly": function() {
			assert.isTrue(this.context.tree === this.context.commit.getTree());
		}
	},
	
	"Setting commit tree *synchronously*": {
		topic: function() {
			var commit = this.context.commit = repo.createCommit();
			var tree = this.context.tree = repo.getTree(fixtureValues.FIRST_TREE.id);
			commit.setTree(tree);
			return true;
		},
		
		"sets tree correctly": function() {
			assert.isTrue(this.context.tree === this.context.commit.getTree());
		}
	},
	
	"Adding a parent commit *asynchronously*": {
		topic: function() {
			var commit = this.context.commit = repo.createCommit();
			var parent = this.context.parent = repo.getCommit(fixtureValues.FIRST_COMMIT.id);
			
			commit.addParent(parent, this.callback);
		},
		
		"adds parent correctly": function(res) {
			assert.isTrue(res);
			assert.isTrue(this.context.commit.getParent(0) === this.context.parent);
		}
	},
	
	"Adding a parent commit *synchronously*": {
		topic: function() {
			var commit = this.context.commit = repo.createCommit();
			var parent = this.context.parent = repo.getCommit(fixtureValues.FIRST_COMMIT.id);

			commit.addParent(parent);
			return true;
		},
		
		"adds parent correctly": function() {
			assert.isTrue(this.context.commit.getParent(0) === this.context.parent);
		}
	},
	
	"Newly created commit": {
		topic: function() {
			return repo.createCommit();
		},
		
		"id is immutable": function(commit) {
			commit.id = "foo";
			assert.isNull(commit.id);
		},

		"saving the Commit gives us an error": function(commit) {
			assert.throws(function() { commit.save(); }, Error);
		},
	},
	
	"Saving a commit *synchronously*": {
		topic: function(commit) {
			var commit = repo.createCommit();
			
			commit.message = "Test commit from Gitteh.";
			commit.author = commit.committer = {
				name: "Sam Day",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			commit.setTree(repo.getTree(fixtureValues.FIRST_TREE.id));

			this.context.commit = commit;
			
			return function() {
				commit.save();
			};
		},
		
		"save works": function(fn) {
			assert.doesNotThrow(fn, Error);
		},
		
		"commit object is not redundant": function() {
			var commit = this.context.commit;
			assert.isTrue(commit === repo.getCommit(commit.id));
		}
	},
	
	"Saving a commit *asynchronously*": {
		topic: function(commit) {
			var commit = repo.createCommit();
			
			commit.message = "Test async commit save from Gitteh.";
			commit.author = commit.committer = {
				name: "Sam Day",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			commit.setTree(repo.getTree(fixtureValues.FIRST_TREE.id));

			this.context.commit = commit;
			
			commit.save(this.callback);
		},
		
		"save works": function(res) {
			assert.isTrue(res);
		},
		
		"commit object is not redundant": function() {
			var commit = this.context.commit;
			assert.isTrue(commit === repo.getCommit(commit.id));
		}
	},
	
	"Creating a Commit and adding a parent by id": {
		topic: function() {
			repo.createCommit(this.callback);
		},
		
		"works": function(commit) {
			assert.doesNotThrow(function() {
				commit.addParent(fixtureValues.FIRST_COMMIT.id);
			}, Error);
			
			assert.equal(commit.parentCount, 1);
		},
		
		"adds correctly": function(commit) {
			assert.isTrue(commit.getParent(0) === repo.getCommit(fixtureValues.FIRST_COMMIT.id));
		}
	}
}).export(module);