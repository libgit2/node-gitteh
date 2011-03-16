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

var createTagTestContext = function(topic, tagFixture) {
	var context = {
		topic: topic,
		
		"tag exists": function(tag) {
			assert.isTrue(!!tag);
		},
		
		"tag is not redundant": function(tag) {
			assert.isTrue(tag === repo.getTag(tagFixture.id));
		},
		
		"has correct *id*": function(tag) {
			assert.equal(tag.id, tagFixture.id);
		},
		
		"*id* is immutable": function(tag) {
			tag.id = "foo";
			assert.equal(tag.id, tagFixture.id);
		},
		
		"has correct *name*": function(tag) {
			assert.equal(tag.name, tagFixture.name);
		},
		
		"has correct *message*": function(tag) {
			assert.equal(tag.message, tagFixture.message);
		},
		
		"has correct *tagger*": function(tag) {
			assert.equal(tag.tagger.name, tagFixture.tagger.name);
			assert.equal(tag.tagger.email, tagFixture.tagger.email);
			assert.equal(tag.tagger.time.getTime(), new Date(tagFixture.tagger.time).getTime());
		},
		
		"has correct *targetId*": function(tag) {
			assert.equal(tag.targetId, tagFixture.targetId);
		},
		
		"has correct target *type*": function(tag) {
			assert.equal(tag.targetType, tagFixture.targetType);
		}
	}
	
	return context;
};

var createSyncTagTestContext = function(tagFixture) {
	return createTagTestContext(function() {
		return repo.getTag(tagFixture.id);
	}, tagFixture);
};

var createAsyncTagTestContext = function(tagFixture) {
	return createTagTestContext(function() {
		repo.getTag(tagFixture.id, this.callback);
	}, tagFixture);
};

vows.describe("Tag").addBatch({
	"Tag *test_tag*, *asynchronously*": createAsyncTagTestContext(fixtureValues.TEST_TAG),
	"Tag *test_tag*, *synchronously*": createSyncTagTestContext(fixtureValues.TEST_TAG),
	
	"Creating a new tag *synchronously*": {
		topic: function() {
			return repo.createTag();
		},

		"tag is in identity state": function(tag) {
			assert.isNull(tag.id);
			assert.isNull(tag.name);
			assert.isNull(tag.message);
			assert.isNull(tag.tagger);
			assert.isNull(tag.targetId);
			assert.isNull(tag.targetType);
		},
	},
	
	"Creating a new tag *asynchronously*": {
		topic: function() {
			repo.createTag(this.callback);
		},

		"tag is in identity state": function(tag) {
			assert.isNull(tag.id);
			assert.isNull(tag.name);
			assert.isNull(tag.message);
			assert.isNull(tag.tagger);
			assert.isNull(tag.targetId);
			assert.isNull(tag.targetType);
		},
	},
		
	"saving identity results in an error": function(tag) {
		var tag = repo.createTag();
		assert.throws(function() {
			tag.save();
		}, Error);
	},
		
	"Saving tag *asynchronously*": {
		topic: function(tag) {
			var tag = repo.createTag();
			tag.name = "Async Test Tag";
			tag.message = "Async Test tag.";
			tag.tagger = {
				name: "Sam Day",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			tag.targetId = fixtureValues.FIRST_COMMIT.id;
			
			this.context.tag = tag;
			
			tag.save(this.callback);
		},
		
		"works": function(result) {
			assert.isTrue(result);
		},
		
		"tag has correct id": function() {
			assert.equal(this.context.tag.id, "19d361418f74421ab4b2baddb9619ea0449a8bdb");
		},
		
		"tag type is correct": function() {
			assert.equal(this.context.tag.targetType, "commit");
		},

		"tag is reachable from repo": function() {
			assert.isTrue(this.context.tag === repo.getTag(this.context.tag.id));
		}
	},
		
	"Saving tag *synchronously*": {
		topic: function(tag) {
			var tag = repo.createTag();
			tag.name = "Sync Test Tag";
			tag.message = "Sync Test tag.";
			tag.tagger = {
				name: "Sam Day",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			tag.targetId = fixtureValues.FIRST_COMMIT.id;
			
			this.context.tag = tag;
			
			return function() {
				tag.save();
			};
		},
		
		"works": function(fn) {
			assert.doesNotThrow(fn, Error);
		},
		
		"tag has correct id": function() {
			assert.equal(this.context.tag.id, "a912c342ece537e69f60352eae2f717a7f72ab2d");
		},
		
		"tag type is correct": function() {
			assert.equal(this.context.tag.targetType, "commit");
		},

		"tag is reachable from repo": function() {
			assert.isTrue(this.context.tag === repo.getTag(this.context.tag.id));
		}
	}
}).export(module);
