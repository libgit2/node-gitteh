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
			repo.createSymbolicReference("refs/heads/asyncsymtest", "refs/heads/master", this.callback);
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
			assert.isTrue(ref === repo.getReference("refs/heads/asyncsymtest"));
		},

		"new ref resolves to same place as refs/heads/master does": function(ref) {
			assert.isTrue(ref.resolve() === repo.getReference("refs/heads/master").resolve());
		}
	},
	
	"Creating a symbolic ref *synchronously*": {
		topic: repo.createSymbolicReference("refs/heads/syncsymtest", "refs/heads/master"),
		
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
			assert.isTrue(ref === repo.getReference("refs/heads/syncsymtest"));
		},

		"new ref resolves to same place as refs/heads/master does": function(ref) {
			assert.isTrue(ref.resolve() === repo.getReference("refs/heads/master").resolve());
		}
	},

	"Creating an OID ref *asynchronously*": {
		topic: function() {
			repo.createOidReference("refs/heads/asyncoidtest", fixtureValues.FIRST_COMMIT.id, this.callback);
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
			assert.equal(ref.target, fixtureValues.FIRST_COMMIT.id);
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === repo.getReference("refs/heads/asyncoidtest"));
		},
	},

	"Creating an OID ref *synchronously*": {
		topic: repo.createOidReference("refs/heads/syncoidtest", fixtureValues.FIRST_COMMIT.id),

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
			assert.equal(ref.target, fixtureValues.FIRST_COMMIT.id);
		},

		"ref is reachable from repository": function(ref) {
			assert.isTrue(ref === repo.getReference("refs/heads/syncoidtest"));
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
			var ref = this.context.ref = repo.createSymbolicReference("refs/heads/settargetasync", "refs/heads/test");
			ref.setTarget("refs/heads/master", this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, "refs/heads/master");
		}
	},
	
/*	"Setting sym target *synchronously*": {
		topic: function() {
			var ref = this.context.ref = repo.createSymbolicReference("refs/heads/settargetasync", "refs/heads/test");
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
			var ref = this.context.ref = repo.createOidReference("refs/heads/setoidasync", fixtureValues.FIRST_COMMIT.id);
			ref.setTarget(fixtureValues.SECOND_COMMIT.id, this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, fixtureValues.SECOND_COMMIT.id);
		}
	},
	
	"Setting oid target *synchronously*": {
		topic: function() {
			var ref = this.context.ref = repo.createOidReference("refs/heads/setoidsync", fixtureValues.FIRST_COMMIT.id);
			return ref.setTarget(fixtureValues.SECOND_COMMIT.id);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"target updated successfully": function() {
			assert.equal(this.context.ref.target, fixtureValues.SECOND_COMMIT.id);
		}
	},
	
	/*"Creating a ref then deleting it": {
		topic: function() {
			var t = this;
			return function() {
				var ref = repo.createOidReference("refs/heads/oidtest", fixtureValues.FIRST_COMMIT.id);
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
	
	/*"Creating an OID ref then changing target": {
		topic: function() {
			var t = this;
			return function() {
				var ref = repo.createOidReference("refs/heads/oidchangetargettest", fixtureValues.SECOND_COMMIT.id);
				console.log(ref);
				ref.setTarget(fixtureValues.SECOND_COMMIT.id);
				
				t.context.ref = ref;
			};
		},
		
		"runs ok": function(fn) {
			fn();
			assert.doesNotThrow(fn, Error);
		},
		
		"updates target correctly": function() {
			assert.equal(this.context.ref.target, fixtureValues.SECOND_COMMIT.id);
		}
	},*/
	
	/*"Renaming a ref *asynchronously*": {
		topic: function() {
			var ref = this.context.ref = repo.createSymbolicReference("refs/heads/asyncrenametest_renameme", "refs/heads/master");
			ref.rename("refs/heads/asyncrenametest", this.callback);
		},
		
		"runs ok": function(result) {
			assert.isTrue(result);
		},
		
		"name is correct": function() {
			assert.equal(this.context.ref.name, "refs/heads/asyncrenametest");
		}
	},
	
	"Renaming a ref *synchronously*": {
		topic: function() {
			var ref = this.context.ref = repo.createSymbolicReference("refs/heads/asyncrenametest_renameme", "refs/heads/master");
			return function() {
				ref.rename("refs/heads/asyncrenametest");
			};
		},
		
		"runs ok": function(fn) {
			try {fn(); } catch(e) {console.log(e);}
			assert.doesNotThrow(fn, Error);
		},
		
		"name is correct": function() {
			assert.equal(this.context.ref.name, "refs/heads/asyncrenametest");
		}
	},*/
	
	"Creating a symbolic ref then changing target": {
		topic: function() {
			var t = this;
			return function() {
				var ref = repo.createSymbolicReference("refs/heads/symtest", "refs/heads/master");
				ref.setTarget("refs/heads/test");

				t.context.ref = ref;
			};
		},
		
		"runs ok": function(fn) {
			assert.doesNotThrow(fn, Error);
		},
		
		"updates target correctly": function() {
			assert.equal(this.context.ref.target, "refs/heads/test");
		}
	},
	
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
		}
	}
})
.addBatch({
	/*"Creating a ref": {
		topic: repo.createSymbolicReference("refs/heads/old", "refs/heads/master"),
		
		"and then renaming it": {
			topic: function(ref) {
				// Make sure the ref we're renaming to doesn't exist.
				try { repo.getReference("refs/heads/new").delete(); } catch(e) {}
				
				ref.rename("refs/heads/new");
				return ref;
			},
			
			"gives us ref with correct name": function(ref) {
				assert.equal(ref.name, "refs/heads/new");
			},
			
			"new name is reachable from repo": function(ref) {
				assert.isTrue(ref === repo.getReference("refs/heads/new"));
			}
		}
	}*/
}).export(module);
