var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
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
	"Tag *test_tag*": createTagTestContext(fixtureValues.TEST_TAG)
}).export(module);
