var gitteh = require("gitteh"), 
	path = require("path"),
	fs = require("fs");

var repository = new gitteh.Repository(path.join(__dirname, "..", ".git"));

var newCommit = repository.createCommit();

// A commit starts off in something of in an "identity" state. Pretty much 
// everything is null. We have to populate the data before a save can happen.

commit.author = commit.committer = {
	name: "Sam Day",
	email: "sam.c.day@gmail.com",
	time: new Date()
};

commit.message = "A demo commit.";

// A commit needs to have a Tree associated with it. This contains the actual 
// files that make up the commit.
var tree = repository.createTree();

// Each tree entry references another git object. Generally a tree entry will
// either reference a "blob", which is an actual file, or it will reference 
// another tree, which is a subdirectory.

// In this case, 
tree.addEntry();
