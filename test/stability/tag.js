// Create a commit, then create a few thousand tags pointing to that commit
// asynchronously.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

var repoPath = path.join(__dirname, "tagrepo/");
try {
	require("wrench").rmdirSyncRecursive(repoPath);
} catch(e) {}
var repo = gitteh.initRepository(repoPath);

var blobRawObj = repo.createRawObject();
blobRawObj.type = "blob";
blobRawObj.data = new Buffer("Hello world!");
blobRawObj.save();

var tree = repo.createTree();
tree.addEntry(blobRawObj.id, "test", 1);
tree.save();

var commit = repo.createCommit();
commit.message = "Test commit.";
commit.setTree(tree);
commit.author = commit.committer = {
	name: "Sam",
	email: "foo@hack.com",
	time: new Date()
};
commit.save();
console.log(commit.id);

var createTag = function(i, tagCallback) {
	var tag_ = null;
	
	async.waterfall([
		function(callback) {
			repo.createTag(callback);
		},
		
		function(tag, callback) {
			tag_ = tag;

			tag.name = "v0." + i;
			tag.message = "Test tag.";
			tag.target = commit.id;
			tag.tagger = {
				name: "Sam",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			
			tag.save(callback);
		},
		
		function(result, callback) {
			if(!result) callback(new Error("Couldn't save Tag."));
			callback(null, tag);
		}
	], function(err) {
		if(err) tagCallback(err);
	});
};

var fns = [];
var num = 10000;
for(var i = 0; i < num; i++) {
	fns.push(createTag.bind(null, i));
}

var start = Date.now();
if(process.argv[1] == __filename) {
	async.parallel(fns, function(err, tags) {
		if(err) {
			console.log("ERROR!");
			throw err;
		}

		var time = Date.now() - start;
		console.log("Took " + time + "ms");
		console.log("Average " + (time/num) + "ms per tag.");
	});
}
