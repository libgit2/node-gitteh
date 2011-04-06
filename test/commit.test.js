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
	fixtureValues = require("./fixtures/values"),
	helpers = require("./fixtures/helpers");

var repo = gitteh.openRepository(fixtureValues.REPO_PATH);
var tempRepo = helpers.createTestRepo("commit");

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
			assert.equal(commit.tree, commitFixture.tree);
		}
	};
	
	if(commitFixture.parents) {
		context["has correct number of parents"] = function(commit) {
			assert.length(commit.parents, commitFixture.parents.length);
		};
		
		context["parents are correct"] = function(commit) {
			commitFixture.parents.forEach(function(commitFixtureParent, i) {
				assert.equal(commit.parents[i], commitFixtureParent);
			});
		};
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

	"Creating a new root commit *asynchronously*": {
		topic: function() {
			var sig = {
				name: "Sam",
				email: "sam@test.com",
				time: new Date(1999, 1, 1)
			};

			
			tempRepo.createCommit({
				message: "Test async commit.",
				author: sig,
				committer: sig,
				tree: tempRepo.TEST_TREE
			}, this.callback);
		},
		
		"gives us the new commit": function(commit) {
			assert.isTrue(!!commit);
		},
		
		"with correct values": function(commit) {
			assert.equal(commit.message, "Test async commit.");
			assert.equal(commit.author.name, "Sam");
			assert.equal(commit.author.email, "sam@test.com");
			assert.equal(commit.author.time.getTime(), new Date(1999, 1, 1).getTime());
			assert.equal(commit.committer.name, "Sam");
			assert.equal(commit.committer.email, "sam@test.com");
			assert.equal(commit.committer.time.getTime(), new Date(1999, 1, 1).getTime());
			assert.equal(commit.tree, tempRepo.TEST_TREE);
		}
	},

	"Creating a new root commit *synchronously*": {
		topic: function() {
			var sig = {
				name: "Sam",
				email: "sam@test.com",
				time: new Date(1999, 1, 1)
			};
 
			return tempRepo.createCommit({
				message: "Test sync commit.",
				author: sig,
				committer: sig,
				tree: tempRepo.TEST_TREE
			});
		},
		
		"gives us the new commit": function(commit) {
			assert.isTrue(!!commit);
		},
		
		"with correct values": function(commit) {
			assert.equal(commit.message, "Test sync commit.");
			assert.equal(commit.author.name, "Sam");
			assert.equal(commit.author.email, "sam@test.com");
			assert.equal(commit.author.time.getTime(), new Date(1999, 1, 1).getTime());
			assert.equal(commit.committer.name, "Sam");
			assert.equal(commit.committer.email, "sam@test.com");
			assert.equal(commit.committer.time.getTime(), new Date(1999, 1, 1).getTime());
			assert.equal(commit.tree, tempRepo.TEST_TREE);
		}
	},
	
	"Saving an existing commit *asynchronously*": {
		topic: function() {
			var sig = {
				name: "Sam",
				email: "sam@test.com",
				time: new Date(1999, 1, 1)
			};
 
			var commit = this.context.commit = tempRepo.createCommit({
				message: "Test async commit to be changed.",
				author: sig,
				committer: sig,
				tree: tempRepo.TEST_TREE
			});
			
			commit.message = "Test async commit changed.";
			commit.save(this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		},
		
		"commit is identical to that retrieved from repository": function() {
			assert.isTrue(this.context.commit === tempRepo.getCommit(this.context.commit.id));
		}
	},
	
	"Saving an existing commit *synchronously*": {
		topic: function() {
			var sig = {
				name: "Sam",
				email: "sam@test.com",
				time: new Date(1999, 1, 1)
			};
 
			var commit = this.context.commit = tempRepo.createCommit({
				message: "Test async commit to be changed.",
				author: sig,
				committer: sig,
				tree: tempRepo.TEST_TREE
			});
			
			commit.message = "Test async commit changed.";
			return commit.save();
		},
		
		"works": function(res) {
			assert.isTrue(res);
		},
		
		"commit is identical to that retrieved from repository": function() {
			assert.isTrue(this.context.commit === tempRepo.getCommit(this.context.commit.id));
		}
	}
}).export(module);