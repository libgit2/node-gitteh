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
var testRepo = helpers.createTestRepo("refs");

vows.describe("References").addBatch({
	"Getting HEAD ref *asynchronously*": {
		topic: function() {
			repo.getReference("HEAD", this.callback);
		},
		
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
	
	"Getting HEAD ref *synchronously*": {
		topic: function() {
			return repo.getReference("HEAD");
		},
		
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
	
	"Creating a symbolic ref *asynchronously*": {
		topic: function() {
			testRepo.createSymbolicReference("refs/heads/asyncsymtest", "refs/heads/master", this.callback);
		},
		
		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"with correct name": function(ref) {
			assert.equal(ref.name, "refs/heads/asyncsymtest");
		},

		"and correct type": function(ref) {
			assert.equal(ref.type, gitteh.GIT_REF_SYMBOLIC);
		},

		"correct target": function(ref) {
			assert.equal(ref.target, "refs/heads/master");
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === testRepo.getReference("refs/heads/asyncsymtest"));
		},

		"new ref resolves to same ref as refs/heads/master does": function(ref) {
			assert.isTrue(ref.resolve() === testRepo.getReference("refs/heads/master").resolve());
		}
	},
	
	"Creating a symbolic ref *synchronously*": {
		topic: testRepo.createSymbolicReference("refs/heads/syncsymtest", "refs/heads/master"),
		
		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"with correct name": function(ref) {
			assert.equal(ref.name, "refs/heads/syncsymtest");
		},

		"and correct type": function(ref) {
			assert.equal(ref.type, gitteh.GIT_REF_SYMBOLIC);
		},

		"correct target": function(ref) {
			assert.equal(ref.target, "refs/heads/master");
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === testRepo.getReference("refs/heads/syncsymtest"));
		},

		"new ref resolves to same place as refs/heads/master does": function(ref) {
			assert.isTrue(ref.resolve() === testRepo.getReference("refs/heads/master").resolve());
		}
	},

	"Creating an OID ref *asynchronously*": {
		topic: function() {
			testRepo.createOidReference("refs/heads/asyncoidtest", testRepo.TEST_COMMIT, this.callback);
		},

		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},

		"with correct name": function(ref) {
			assert.equal(ref.name, "refs/heads/asyncoidtest");
		},

		"and correct type": function(ref) {
			assert.equal(ref.type, gitteh.GIT_REF_OID);
		},

		"and correct target": function(ref) {
			assert.equal(ref.target, testRepo.TEST_COMMIT);
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === testRepo.getReference("refs/heads/asyncoidtest"));
		},
	},

	"Creating an OID ref *synchronously*": {
		topic: testRepo.createOidReference("refs/heads/syncoidtest", testRepo.TEST_COMMIT),

		"gives us a reference": function(ref) {
			assert.isTrue(!!ref);
		},

		"with correct name": function(ref) {
			assert.equal(ref.name, "refs/heads/syncoidtest");
		},

		"and correct type": function(ref) {
			assert.equal(ref.type, gitteh.GIT_REF_OID);
		},

		"and correct target": function(ref) {
			assert.equal(ref.target, testRepo.TEST_COMMIT);
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === testRepo.getReference("refs/heads/syncoidtest"));
		},
	},
	
	"Resolving HEAD *asynchronously*": {
		topic: function() {
			var ref = repo.getReference("HEAD");
			ref.resolve(this.callback);
		},
		
		"gives us a ref": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"is refs/heads/master": function(ref) {
			assert.equal(ref.name, "refs/heads/master");
		}
	},
	
	"Resolving HEAD *synchronously*": {
		topic: function() {
			var ref = repo.getReference("HEAD");
			return ref.resolve();
		},
		
		"gives us a ref": function(ref) {
			assert.isTrue(!!ref);
		},
		
		"is refs/heads/master": function(ref) {
			assert.equal(ref.name, "refs/heads/master");
		}
	},
	
	"Setting sym target *asynchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createSymbolicReference("refs/heads/settargetasync", "refs/heads/test");
			ref.setTarget("refs/heads/master", this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, "refs/heads/master");
		}
	},
	
	"Setting sym target *synchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createSymbolicReference("refs/heads/settargetsync", "refs/heads/test");
			return ref.setTarget("refs/heads/master");
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, "refs/heads/master");
		}
	},
	
	"Setting oid target *asynchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createOidReference("refs/heads/setoidasync", testRepo.TEST_COMMIT);
			ref.setTarget(testRepo.HEAD_COMMIT, this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, testRepo.HEAD_COMMIT);
		}
	},
	
	"Setting oid target *synchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createOidReference("refs/heads/setoidsync", testRepo.TEST_COMMIT);
			return ref.setTarget(testRepo.HEAD_COMMIT);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, testRepo.HEAD_COMMIT);
		}
	},
	
	"Renaming a ref *asynchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createSymbolicReference("refs/heads/asyncrenametest_renameme", "refs/heads/master");
			ref.rename("refs/heads/asyncrenametest_renamed", this.callback);
		},
		
		"runs ok": function(result) {
			assert.isTrue(result);
		},
		
		"name is correct": function() {
			assert.equal(this.context.ref.name, "refs/heads/asyncrenametest_renamed");
		}
	},
	
	"Renaming a ref *synchronously*": {
		topic: function() {
			var ref = this.context.ref = testRepo.createSymbolicReference("refs/heads/syncrenametest_renameme", "refs/heads/master");
			return function() {
				ref.rename("refs/heads/syncrenametest_renamed");
			};
		},
		
		"runs ok": function(fn) {
			assert.doesNotThrow(fn, Error);
		},
		
		"name is correct": function() {
			assert.equal(this.context.ref.name, "refs/heads/syncrenametest_renamed");
		}
	},

	/*"Creating a ref then deleting it *asynchronously*": {
		topic: function() {
			var t = this;
			return function() {
				var ref = repo.createOidReference("refs/heads/deleteme", testRepo.HEAD_COMMIT);
				ref.delete();
				t.context.ref = ref;
			};
		},
		
		"runs fine": function(fn) {
			assert.doesNotThrow(fn);
		},
		
		"ref is completely inoperable now": function() {
			var ref = this.context.ref;
			assert.throws(function() {
				ref.rename("refs/heads/oidtest");
			}, Error);
			
			assert.throws(function() {
				ref.resolve();
			});
			
			assert.throws(function() {
				ref.delete();
			});
			
			assert.throws(function() {
				ref.setTarget(fixtureValues.FIRST_COMMIT.id);
			});
		}
	},*/
	
	
	"Loading reference list *asynchronously*": {
		topic: function() {
			repo.listReferences(gitteh.GIT_REF_LISTALL, this.callback);
		},
		
		"gives us an Array": function(refs) {
			assert.isArray(refs);
		}
	},
	
	"Loading reference list *synchronously*": {
		topic: function() {
			return repo.listReferences(gitteh.GIT_REF_LISTALL);
		},
		
		"gives us an Array": function(refs) {
			assert.isArray(refs);
		},
		
		"has correct elements": function(refs) {
			assert.equal(refs[0], "refs/heads/master");
			assert.equal(refs[1], "refs/tags/test_tag");
			assert.equal(refs[2], "refs/heads/test");
		}
	}
}).export(module);
