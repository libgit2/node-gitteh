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
var tempRepo = helpers.createTestRepo();

vows.describe("Blob").addBatch({
	teardown: function() {
		helpers.cleanupTestRepo(tempRepo);
	},

	"Getting a blob *asynchronously*": {
		topic: function() {
			repo.getBlob(fixtureValues.TEST_BLOB, this.callback);
		},
		
		"gives us a Blob": function(blob) {
			assert.isTrue(!!blob);
		},
		
		"with the correct data": function(blob) {
			assert.equal(blob.data.toString(), "Hello world!\n\n");
		}
	},

	"Getting a blob *synchronously*": {
		topic: function() {
			return repo.getBlob(fixtureValues.TEST_BLOB);
		},
		
		"gives us a Blob": function(blob) {
			assert.isTrue(!!blob);
		},
		
		"with the correct data": function(blob) {
			assert.equal(blob.data.toString(), "Hello world!\n\n");
		}
	},
	
	"Creating a new blob *asynchronously*": {
		topic: function() {
			tempRepo.createBlob({
				data: new Buffer("Asynchronous blob creation test.")
			}, this.callback);
		},
		
		"successfully creates a blob": function(blob) {
			assert.isTrue(!!blob);
		},
		
		"with the correct id": function(blob) {
			assert.equal(blob.id, helpers.getSHA1("blob 32\0Asynchronous blob creation test."));
		}
	},
	
	"Creating a new blob *synchronously*": {
		topic: function() {
			tempRepo.createBlob({
				data: new Buffer("Synchronous blob creation test.")
			}, this.callback);
		},
		
		"successfully creates a blob": function(blob) {
			assert.isTrue(!!blob);
		},
		
		"with the correct id": function(blob) {
			assert.equal(blob.id, helpers.getSHA1("blob 31\0Synchronous blob creation test."));
		}
	},

	"Modifying a blob *asynchronously*": {
		topic: function() {
			var blob = this.context.blob = tempRepo.createBlob({
				data: new Buffer("Modifying blob async.")
			});
			
			blob.data = new Buffer("Modified blob async.");
			blob.save(this.callback);
		},

		"works": function(res) {
			assert.isTrue(res);
		},
		
		"updates blob ID correctly": function() {
			assert.equal(this.context.blob.id, helpers.getSHA1("blob 20\0Modified blob async."))
		}
	},

	"Modifying a blob *synchronously*": {
		topic: function() {
			var blob = tempRepo.createBlob({
				data: new Buffer("Modifying blob sync.")
			});
			
			blob.data = new Buffer("Modified blob sync.");
			return blob;
		},

		"works": function(blob) {
			assert.isTrue(blob.save());
		},
		
		"updates blob ID correctly": function(blob) {
			assert.equal(blob.id, helpers.getSHA1("blob 19\0Modified blob sync."))
		}
	}
}).export(module);