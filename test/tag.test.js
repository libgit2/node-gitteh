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
var testRepo = helpers.createTestRepo("tag");

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
	
	"Creating a new tag *asynchronously*": {
		topic: function() {
			testRepo.createTag({
				name: "async_test_tag",
				message: "Async Test tag.",
				tagger: {
					name: "Sam Day",
					email: "sam.c.day@gmail.com",
					time: new Date(1988, 12, 12)
				},
				targetId: testRepo.TEST_BLOB
			}, this.callback);
		},

		"gives us a tag": function(tag) {
			assert.isTrue(!!tag);
		},
		
		"with correct values": function(tag) {
			assert.equal(tag.name, "async_test_tag");
			assert.equal(tag.message, "Async Test tag.");
			assert.equal(tag.tagger.name, "Sam Day");
			assert.equal(tag.tagger.email, "sam.c.day@gmail.com");
			assert.equal(tag.tagger.time.getTime(), new Date(1988, 12, 12).getTime());
			assert.equal(tag.targetId, testRepo.TEST_BLOB);
			assert.equal(tag.targetType, "blob");
		}
	},
	
	"Creating a new tag *synchronously*": {
		topic: function() {
			return testRepo.createTag({
				name: "sync_test_tag",
				message: "Sync Test tag.",
				tagger: {
					name: "Sam Day",
					email: "sam.c.day@gmail.com",
					time: new Date(1988, 12, 12)
				},
				targetId: testRepo.TEST_BLOB
			});
		},

		"gives us a tag": function(tag) {
			assert.isTrue(!!tag);
		},
		
		"with correct values": function(tag) {
			assert.equal(tag.name, "sync_test_tag");
			assert.equal(tag.message, "Sync Test tag.");
			assert.equal(tag.tagger.name, "Sam Day");
			assert.equal(tag.tagger.email, "sam.c.day@gmail.com");
			assert.equal(tag.tagger.time.getTime(), new Date(1988, 12, 12).getTime());
			assert.equal(tag.targetId, testRepo.TEST_BLOB);
			assert.equal(tag.targetType, "blob");
		}
	}
}).export(module);
