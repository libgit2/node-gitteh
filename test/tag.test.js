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

var createTagTestContext = function(tagFixture) {
	var context = {
		topic: function() {
			repo.getTag(tagFixture.id, this.callback);
		},
		
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

vows.describe("Tag").addBatch({
	"Tag *test_tag*": createTagTestContext(fixtureValues.TEST_TAG),
	
	"Creating a new tag": {
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
		
		"saving identity results in an error": function(tag) {
			assert.throws(function() {
				tag.save();
			}, Error);
		},
		
		"setting valid data and saving works": function(tag) {
			tag.name = "Test Tag";
			tag.message = "Test tag.";
			tag.tagger = {
				name: "Sam Day",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			tag.targetId = fixtureValues.FIRST_COMMIT.id;
			
			tag.save();
		},
		
		"tag has correct id": function(tag) {
			assert.equal(tag.id, "b76986bf57110b466b2f77ef662ea37f9d5eab80");
		},
		
		"tag type is correct": function(tag) {
			assert.equal(tag.targetType, "commit");
		},

		"tag is reachable from repo": function(tag) {
			assert.isTrue(tag === repo.getTag(tag.id));
		}
	}
}).export(module);
