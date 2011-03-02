var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);

vows.describe("RevWalker").addBatch({
	"RevWalker from second commit": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TIME);
			return walker;
		},
	
		"gives us a walker": function(walker) {
			assert.isTrue(!!walker);
		},
		
		"pushing second commit works": function(walker) {
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
		},
		
		"calling *next()* gives us second commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
		},
		
		"calling *next()* gives us first commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
		},
		
		"calling *next()* gives us null.": function(walker) {
			var commit = walker.next();
			assert.isNull(commit);
		}
	},

	"RevWalker from second commit in reverse order": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TIME | gitteh.SORT_REVERSE);
			walker.push(repo.getCommit(fixtureValues.SECOND_COMMIT.id));
			return walker;
		},
	
		"gives us a walker": function(walker) {
			assert.isTrue(!!walker);
		},

		"calling *next()* gives us first commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIRST_COMMIT.id);
		},
		
		"calling *next()* gives us second commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.SECOND_COMMIT.id);
		},
		
		"calling *next()* gives us null.": function(walker) {
			var commit = walker.next();
			assert.isNull(commit);
		}
	},
	
	"RevWalker from fifth commit in topographical order": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TOPOLOGICAL);
			walker.push(repo.getCommit(fixtureValues.FIFTH_COMMIT.id));
			return walker;
		},

		"calling *next()* gives us fifth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIFTH_COMMIT.id);
		},

		"calling *next()* gives us fourth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FOURTH_COMMIT.id);
		},

		"calling *next()* gives us third commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.THIRD_COMMIT.id);
		}
	},
	
	"RevWalker from fifth commit in topographical order": {
		topic: function() {
			var walker = repo.createWalker();
			walker.sort(gitteh.SORT_TOPOLOGICAL);
			walker.push(repo.getCommit(fixtureValues.FIFTH_COMMIT.id));
			return walker;
		},

		"calling *next()* gives us fifth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FIFTH_COMMIT.id);
		},

		"calling *next()* gives us fourth commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.FOURTH_COMMIT.id);
		},

		"calling *next()* gives us third commit.": function(walker) {
			var commit = walker.next();
			assert.equal(commit.id, fixtureValues.THIRD_COMMIT.id);
		}
	}
}).export(module);
