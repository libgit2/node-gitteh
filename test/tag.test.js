var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(fixtureValues.REPO_PATH); 

var createTagTestContext = function(tagFixture) {
	var context = {
		topic: function() {
			var tag = repo.getTag(tagFixture.id);
			return tag;
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
