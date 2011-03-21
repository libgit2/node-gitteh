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
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = gitteh.openRepository(fixtureValues.REPO_PATH);
var workingRepo = gitteh.openRepository(path.join(__dirname, "..", ".git"));

vows.describe("Index").addBatch({
	"Opening index on bare repo *asynchronously*": {
		topic: function() {
			repo.getIndex(this.callback);
		},
		
		"gives us an Index": function(index) {
			assert.isTrue(!!index);
		},
	},
	
	"Opening index on bare repo *synchronously*": {
		topic: function() {
			return repo.getIndex();
		},
		
		"gives us an Index": function(index) {
			assert.isTrue(!!index);
		},
	},
	
	"Opening index on checkout out repo *asynchronously*": {
		topic: function() {
			workingRepo.getIndex(this.callback);
		},
		
		"gives us an Index": function(index) {
			assert.isTrue(!!index);
		},
	},
	
	"Opening index on checkout out repo *synchronously*": {
		topic: function() {
			return workingRepo.getIndex();
		},
		
		"gives us an Index": function(index) {
			assert.isTrue(!!index);
		},
	},
	
	"Getting an index entry from checked out repo *asynchronously*": {
		topic: function() {
			var index = workingRepo.getIndex();
			index.getEntry(0, this.callback);
		},
		
		"gives us an entry": function(entry) {
			assert.isTrue(!!entry);
		}
	},
	
	"Getting an index entry from checked out repo *synchronously*": {
		topic: function() {
			var index = workingRepo.getIndex();
			return index.getEntry(0);
		},
		
		"gives us an entry": function(entry) {
			assert.isTrue(!!entry);
		}
	}
}).export(module);