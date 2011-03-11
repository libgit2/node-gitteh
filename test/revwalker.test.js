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
	"RevWalker from second commit": {
		topic: function() {
			//return repo.createWalker();
			repo.createWalker(this.callback);
		},

		"gives us a walker": function(walker) {
			console.log("woot.");
			assert.isTrue(!!walker);
		},
		
		"pushing second commit works": function(walker) {
			walker.sort(gitteh.SORT_TIME);
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
		},
		
		"calling *next*": { 
			topic: function(walker) {
				var commit = walker.next();
				this.context.walker = walker;
				return commit;
			},
			
			"gives us second commit": function(commit) {
				assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
			},
			
			"commit is not redundant": function(commit) {
				assert.isTrue(commit === repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			},
			
			"calling *next()* gives us first commit.": function() {
				var walker = this.context.walker;
				var commit = walker.next();
				assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
			},
			
			"calling *next()* gives us null.": function() {
				var walker = this.context.walker;
				var commit = walker.next();
				assert.isNull(commit);
			}
		}
	},
/*
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
	}*/
}).export(module);
