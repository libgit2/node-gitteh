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

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);
vows.describe("References").addBatch({
	"Getting HEAD ref": {
		topic: repo.getReference("HEAD"),
		
		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"name is correct": function(ref) {
			assert.equal(ref.name, "HEAD");
		},
		
		"- then resolving it": {
			topic: function(ref) {
				return ref.resolve();
			},
			
			"gives us a ref": function(ref) {
				assert.isTrue(!!ref);
			},
			
			"gives us a non-symbolic ref": function(ref) {
				assert.equal(ref.type, gitteh.GIT_REF_OID);
			},
			
			"gives us the ref pointing to latest commit": function(ref) {
				assert.equal(ref.target, fixtureValues.LATEST_COMMIT.id);
			}
		}
	},
	
	"Creating a symbolic ref": {
		topic: repo.createSymbolicReference("heads/test", "heads/master"),
		
		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"with correct name": function(ref) {
			assert(ref.name, "heads/test");
		},
		
		"and correct type": function(ref) {
			assert(ref.type, gitteh.GIT_REF_SYMBOLIC);
		},
		
		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === repo.getReference("heads/test"));
		}
	}
}).export(module);
