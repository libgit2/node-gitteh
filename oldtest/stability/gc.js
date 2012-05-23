// Tests some situations with the GC that could cause issues.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");
	
var path = "/tmp/gcstress" + Date.now() + "/";
try {
	require("wrench").rmdirSyncRecursive(path);
} catch(e) {}

var repo = gitteh.initRepository(path);

// Create an empty blob.
var blobRawObj = repo.createRawObject();
blobRawObj.type = "blob";
blobRawObj.data = new Buffer("Hello world!");
blobRawObj.save();

var tree = repo.createTree();
tree.addEntry(blobRawObj.id, (Math.random() * 10000) + "", 33188);
tree.save();

for(var i = 0; i < 10000; i++) {
	var commit = repo.createCommit();
	commit.message = "blah";
	commit.author = commit.committer = {
		name: "Sam",
		email: "sam.c.day@gmail.com",
		time: new Date()
	};
	
	commit.setTree(tree);
	commit.save();
}

gc();
return;