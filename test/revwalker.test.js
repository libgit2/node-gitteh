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

vows.describe("RevWalker").addBatch({
	"Creating a RevWalker *asynchronously*": {
		topic: function() {
			repo.createWalker(this.callback);
		},
		
		"gives us a walker": function(walker) {
			assert.isTrue(!!walker);
		}
	},
	
	"Creating a RevWalker *synchronously*": {
		topic: function() {
			return repo.createWalker();
		},

		"gives us a walker": function(walker) {
			assert.isTrue(!!walker);
		}
	},
	
	"Pushing a commit object onto revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.push(repo.getCommit(fixtureValues.FIRST_COMMIT.id), this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Pushing a commit object onto revwalker *synchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.push(repo.getCommit(fixtureValues.FIRST_COMMIT.id), this.callback);
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},
	
	"Pushing a commit ID onto revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.push(fixtureValues.FIRST_COMMIT.id, this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Pushing a commit ID onto revwalker *synchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.push(fixtureValues.FIRST_COMMIT.id, this.callback);
			};
		},

		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},
	
	"Getting a commit from revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			walker.next(this.callback);
		},
		
		"gives us correct commit": function(commit) {
			assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
		},
		
		"commit is not redundant": function(commit) {
			assert.isTrue(commit === repo.getCommit(fixtureValues.SECOND_COMMIT.id));
		}
	},
	
	"Getting a commit from revwalker *synchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			return walker.next();
		},
		
		"gives us correct commit": function(commit) {
			assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
		},
		
		"commit is not redundant": function(commit) {
			assert.isTrue(commit === repo.getCommit(fixtureValues.SECOND_COMMIT.id));
		}
	},
	
	"Hiding a commit id from revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.hide(fixtureValues.SECOND_COMMIT.id, this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Hiding a commit id from revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.hide(fixtureValues.SECOND_COMMIT.id);
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},
	
	"Hiding a commit object from revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.hide(repo.getCommit(fixtureValues.SECOND_COMMIT.id), this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Hiding a commit object from revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.hide(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},
	
	"Sorting a revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TIME, this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Sorting a revwalker *synchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.sort(gitteh.SORT_TIME);
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},
	
	"Resetting a revwalker *asynchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			walker.reset(this.callback);
		},
		
		"works": function(res) {
			assert.isTrue(res);
		}
	},
	
	"Resetting a revwalker *synchronously*": {
		topic: function() {
			var walker = repo.createWalker();
			return function() {
				walker.reset();
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		}
	},

	"RevWalker from second commit in reverse order": {
		topic: function() {
			repo.createWalker(this.callback);
		},
	
		"gives us a walker": function(walker) {
			assert.isTrue(!!walker);
		},

		"calling *next()* gives us first commit.": function(walker) {
			walker.sort(gitteh.SORT_TIME | gitteh.SORT_REVERSE);
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
		},
		
		"calling *next()* gives us second commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
		},
		
		"calling *next()* gives us null.": function(walker) {
			var commit = walker.next();
			assert.isNull(commit);
		}
	},
	
	"RevWalker from fifth commit in topographical order": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TOPOLOGICAL);
			walker.push(repo.getCommit(fixtureValues.FIFTH_COMMIT.id));
			return walker;
		},

		"calling *next()* gives us fifth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIFTH_COMMIT.id);
		},

		"calling *next()* gives us fourth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FOURTH_COMMIT.id);
		},

		"calling *next()* gives us third commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.THIRD_COMMIT.id);
		}
	},
	
	"RevWalker with reset": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TIME);
			walker.push(fixtureValues.SECOND_COMMIT.id);
			walker.reset();
			walker.push(fixtureValues.FIFTH_COMMIT.id);
			return walker;
		},
		
		"gives us correct commit": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIFTH_COMMIT.id);
		}
	},
	
	"RevWalker with hide": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TIME);
			walker.push(fixtureValues.FOURTH_COMMIT.id);
			walker.hide(fixtureValues.THIRD_COMMIT.id);
			walker.next();
			return walker;
		},
		
		"gives us correct commit": function(walker) {
			var commit = walker.next();
			assert.isNull(commit);
		}
	}
}).export(module);
