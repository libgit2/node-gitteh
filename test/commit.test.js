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

var createCommitTests = function(commitFixture) {
	var context = {
		topic: function() {
			repo.getCommit(commitFixture.id, this.callback);
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

vows.describe("Commit").addBatch({
	"First commit": createCommitTests(fixtureValues.FIRST_COMMIT),
	"Second commit": createCommitTests(fixtureValues.SECOND_COMMIT),
	"Third commit": createCommitTests(fixtureValues.THIRD_COMMIT),
	"Fourth commit": createCommitTests(fixtureValues.FOURTH_COMMIT),
	"Fifth commit": createCommitTests(fixtureValues.FIFTH_COMMIT),

	"Creating a new commit": {
		topic: function() {
			repo.createCommit(this.callback);
		},
		
		"gives us an identity Commit": function(commit) {
			assert.isTrue(!!commit);
			assert.isNull(commit.id);
			assert.isNull(commit.message);
			assert.isNull(commit.author);
			assert.isNull(commit.committer);
			assert.isNull(commit.getTree());
		},
		
		"id is immutable": function(commit) {
			commit.id = "foo";
			assert.isNull(commit.id);
		},

		"saving the Commit gives us an error": function(commit) {
			assert.throws(function() { commit.save(); }, Error);
		},
		
		"- setting valid data and saving": {
			topic: function(commit) {
				commit.message = "Test commit from Gitteh.";
				commit.author = commit.committer = {
					name: "Sam Day",
					email: "sam.c.day@gmail.com",
					time: new Date()
				};
				commit.setTree(repo.getTree(fixtureValues.FIRST_TREE.id));

				return function() {
					commit.save();
				};
			},
			
			"save works": function(fn) {
				assert.doesNotThrow(fn, Error);
			},
			
			"commit object is not redundant": function() {
				var commit = this.context.topics[1];
				assert.isTrue(commit === repo.getCommit(commit.id));
			}
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