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
	}
}).export(module);