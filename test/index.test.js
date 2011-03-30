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
	fixtureValues = require("./fixtures/values"),
	fs = require("fs"),
	helpers = require("./fixtures/helpers");

var repo = gitteh.openRepository(fixtureValues.REPO_PATH);
var workingRepo = gitteh.openRepository2(fixtureValues.WORKING_DIR);

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
	},
	
	"Finding an index entry by path *asynchronously*": {
		topic: function() {
			var index = workingRepo.getIndex();
			index.findEntry("file.txt", this.callback);
		},
		
		"gives us an entry": function(entry) {
			assert.isTrue(!!entry);
		},
		
		"with the correct path": function(entry) {
			assert.equal(entry.path, "file.txt");
		}
	},
	
	"Finding an index entry by path *synchronously*": {
		topic: function() {
			var index = workingRepo.getIndex();
			return index.findEntry("file.txt");
		},
		
		"gives us an entry": function(entry) {
			assert.isTrue(!!entry);
		},
		
		"with the correct path": function(entry) {
			assert.equal(entry.path, "file.txt");
		}
	},
	
	"Adding an entry from checked out repo *asynchronously*": {
		topic: function() {
			var repo = this.context.repo = helpers.createTestRepo("asyncindexadd");
			var index = this.context.index = repo.getIndex();
			
			this.context.oldIndexCount = index.entryCount;
			index.addEntry("unstaged.txt", 1, this.callback);
		},
		
		"works correctly": function(res) {
			assert.isTrue(res);
		},
		
		"index entry count incremented": function() {
			assert.equal(this.context.index.entryCount, this.context.oldIndexCount + 1);
		},
		
		"new entry has correct values": function() {
			var entry = this.context.index.getEntry(this.context.oldIndexCount);
			var stats = fs.statSync(path.join(this.context.repo.path, "..", "unstaged.txt"));

			assert.equal(entry.mtime.getTime(), stats.mtime.getTime());
			assert.equal(entry.ctime.getTime(), stats.ctime.getTime());
			assert.equal(entry.ino, stats.ino);
			assert.equal(entry.mode, stats.mode);
			assert.equal(entry.path, "unstaged.txt");
			assert.equal(entry.file_size, stats.size);
		}			
	},
	
	"Adding an entry from checked out repo *synchronously*": {
		topic: function() {
			var repo = this.context.repo = helpers.createTestRepo("syncindexadd");
			var index = this.context.index = repo	.getIndex();
			
			this.context.oldIndexCount = index.entryCount;
			return index.addEntry("unstaged.txt", 1);
		},
		
		"works correctly": function(res) {
			assert.isTrue(res);
		},
		
		"index entry count incremented": function() {
			assert.equal(this.context.index.entryCount, this.context.oldIndexCount + 1);
		},
		
		"new entry has correct values": function() {
			var entry = this.context.index.getEntry(this.context.oldIndexCount);
			var stats = fs.statSync(path.join(this.context.repo.path, "..", "unstaged.txt"));

			assert.equal(entry.mtime.getTime(), stats.mtime.getTime());
			assert.equal(entry.ctime.getTime(), stats.ctime.getTime());
			assert.equal(entry.ino, stats.ino);
			assert.equal(entry.mode, stats.mode);
			assert.equal(entry.path, "unstaged.txt");
			assert.equal(entry.file_size, stats.size);
		}			
	}
}).export(module);