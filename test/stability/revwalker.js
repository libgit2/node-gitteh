// Create a repo with a hundred commits, walk through history in both directions
// simultaneously a few thousand times.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

var path = "/tmp/revwalkstress" + Date.now() + "/";
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

var lastCommit = null;
for(var i = 0; i < 1000; i++) {
	var commit = repo.createCommit();
	commit.message = "blah";
	commit.author = commit.committer = {
		name: "Sam",
		email: "sam.c.day@gmail.com",
		time: new Date()
	};
	
	if(lastCommit) commit.addParent(lastCommit);
	
	commit.setTree(tree);
	commit.save();
		
	lastCommit = commit;
}

repo.createOidReference("refs/heads/master", lastCommit.id);

var traverse = function(walker, callback) {
	walker.next(function(err, commit) {
		if(err) return callback(err);
		else {
			if(commit == null) {
				return callback();
			}

			traverse(walker, callback);
		}
	});
};

var walk = function(callback, sorting) {
	repo.createWalker(function(err, walker) {
		if(err) return callback(err);

		walker.sort(sorting, function(err) {
			if(err) return callback(err);

			walker.push(lastCommit.id, function(err) {
				if(err) return callback(err);
				traverse(walker, callback);
			});
		});
	});
};

var walkTree = function(callback) {
	walk(callback, gitteh.SORT_TIME);
};

var walkTreeReverse = function(callback) {
	walk(callback, gitteh.SORT_TIME | gitteh.SORT_REVERSE);
};

var fns = [];
for(var i = 0; i < 100; i++) {
	fns.push(walkTree);
	fns.push(walkTreeReverse);
}

if(process.argv[1] == __filename) {
	console.log("Starting.");
	async.parallel(fns, function() {
		console.log("All done!");
	});
}
