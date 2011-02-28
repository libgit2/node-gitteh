var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	profiler = require("profiler");

var repo = new gitteh.Repository(path.join(__dirname, "fixtures/gitrepo"));
vows.describe("Commit").addBatch({
	"Commit *ca1f3314acd24c4551da8c26adaf562272db1d19*": {
		topic: function() {
			return repo.getCommit("ca1f3314acd24c4551da8c26adaf562272db1d19");
		},
		
		"exists": function(commit) {
			assert.isNotNull(commit);
		},
		
		"gives us same object if requested again": function(commit) {
			assert.isTrue(commit === repo.getCommit("ca1f3314acd24c4551da8c26adaf562272db1d19"), "Commit objects do not match!");
		},
		
		"has correct id": function(commit) {
			assert.equal(commit.id, "ca1f3314acd24c4551da8c26adaf562272db1d19");
		},
		
		"has correct message": function(commit) {
			assert.equal(commit.message, "First commit.\n");
		},
		
		"has correct short message": function(commit) {
			assert.equal(commit.shortMessage, "First commit.");
		},
		
		"has correct author": function(commit) {
			assert.isNotNull(commit.author);
			assert.equal(commit.author.name, "Sam Day");
			assert.equal(commit.author.email, "sam.c.day@gmail.com");
		},
		
		"has correct committer": function(commit) {
			assert.isNotNull(commit.committer);
			assert.equal(commit.committer.name, "Sam Day");
			assert.equal(commit.committer.email, "sam.c.day@gmail.com");
		},
		
		"has correct time": function(commit) {
			assert.equal(commit.time.getTime(), new Date("Mon Feb 28 14:59:45 2011 +1000").getTime());
		}
	}
}).export(module);