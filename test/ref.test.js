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
	helpers = require("./fixtures/helpers"),
	glob = require("glob"),
	async = require("async");

process.setMaxListeners(100);

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

	"Creating a ref then deleting it *asynchronously*": {
		topic: function() {
			var t = this;
			var ref = testRepo.createOidReference("refs/heads/asyncdeleteme", testRepo.HEAD_COMMIT);
			t.context.ref = ref;
			ref.delete(this.callback);
		},
		
		"runs fine": function(result) {
			assert.isTrue(result);
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
	},

	"Creating a ref then deleting it *synchronously*": {
		topic: function() {
			var t = this;
			return function() {
				var ref = testRepo.createOidReference("refs/heads/syncdeleteme", testRepo.HEAD_COMMIT);
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
	},

	"Loading reference list *asynchronously*": {
		topic: function() {
			repo.listReferences(gitteh.GIT_REF_LISTALL, this.callback);
		},
		
		"gives us an Array": function(refs) {
			assert.isArray(refs);
		},
		
		"has correct elements": function(refs) {
			assert.equal(refs[0], "refs/heads/master");
			assert.equal(refs[1], "refs/tags/test_tag");
			assert.equal(refs[2], "refs/heads/test");
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
	},
	
	"Packing refs *asynchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("asyncrefpack");
			testRepo.packReferences(this.callback);
		},
		
		"runs correctly": function(result) {
			assert.isTrue(result);
		},
		
		"no more loose references": function() {
			assert.length(glob.globSync(path.join(this.context.repo.path, "refs", "heads", "/") + "*"), 0);
		},
		
		"grabbing master ref works": function() {
			var master;
			var that = this;
			assert.doesNotThrow(function() {
				master = that.context.repo.getReference("refs/heads/master");
			}, Error);
			assert.isTrue(!!master);
			assert.equal(master.target, this.context.repo.HEAD_COMMIT);
		}
	},
	
	"Packing refs *synchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("syncrefpack");
			return testRepo.packReferences();
		},
		
		"runs correctly": function(result) {
			assert.isTrue(result);
		},
		
		"no more loose references": function() {
			assert.length(glob.globSync(path.join(this.context.repo.path, "refs", "heads", "/") + "*"), 0);
		},
		
		"grabbing master ref works": function() {
			var master;
			var that = this;
			assert.doesNotThrow(function() {
				master = that.context.repo.getReference("refs/heads/master");
			}, Error);
			assert.isTrue(!!master);
			assert.equal(master.target, this.context.repo.HEAD_COMMIT);
		}
	},
	
	"Packing refs with a ref already open *asynchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("ayncrefpack");

			var ref = this.context.ref = testRepo.getReference("refs/heads/master");
			testRepo.packReferences(this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"no more loose references": function() {
			assert.length(glob.globSync(path.join(this.context.repo.path, "refs", "heads", "/") + "*"), 0);
		},
		
		"ref is still valid": function() {
			var that = this;
			
			assert.doesNotThrow(function() {
				that.context.ref.rename("refs/heads/itworks");
			}, Error);
			
			assert.isTrue(this.context.ref == this.context.repo.getReference("refs/heads/itworks"));
		}
	},
	
	"Packing refs with a ref already open *synchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("ayncrefpack");

			var ref = this.context.ref = testRepo.getReference("refs/heads/master");
			return testRepo.packReferences();
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"no more loose references": function() {
			assert.length(glob.globSync(path.join(this.context.repo.path, "refs", "heads", "/") + "*"), 0);
		},
		
		"ref is still valid": function() {
			var that = this;
			
			assert.doesNotThrow(function() {
				that.context.ref.rename("refs/heads/itworks");
			}, Error);
			
			assert.isTrue(this.context.ref == this.context.repo.getReference("refs/heads/itworks"));
		}
	},
	
	"Packing refs with a ref currently opening *asynchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("ayncrefpack");
			
			async.parallel({
				ref: function(callback) { testRepo.getReference("refs/heads/master", callback); },
				pack: function(callback) { testRepo.packReferences(callback); }
			}, this.callback);
		},
		
		"works": function(results) {
			assert.isTrue(results.pack);
			assert.isTrue(!!results.ref);
		},
		
		"ref is still usable": function(results) {
			assert.doesNotThrow(function() {
				results.ref.rename("refs/heads/renameme");
			}, Error);
			assert.isTrue(results.ref === this.context.repo.getReference("refs/heads/renameme"));
		}
	},

	"Packing refs with a ref currently opening *synchronously*": {
		topic: function() {
			var testRepo = this.context.repo = helpers.createTestRepo("ayncrefpack");
			
			async.parallel({
				ref: function(callback) { testRepo.getReference("refs/heads/master", callback); },
				pack: function(callback) {
					try {
						callback(null, testRepo.packReferences());
					}
					catch(e) {
						callback(e);
					}
				}
			}, this.callback);
		},
		
		"works": function(results) {
			assert.isTrue(results.pack);
			assert.isTrue(!!results.ref);
		},

		"ref is still usable": function(results) {
			assert.doesNotThrow(function() {
				results.ref.rename("refs/heads/renameme");
			}, Error);
			assert.isTrue(results.ref === this.context.repo.getReference("refs/heads/renameme"));
		}
	}
}).export(module);
