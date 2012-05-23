// Step 1, open a repository, step 2, populate it with 1000 commits in an async
// manner. Step 3, iterate over all of these commits and mess with them.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

/*process.on("uncaughtException", function(err) {
	console.log("Uncaught exception!");
	console.log(err);
	throw err;
});*/

var path = "/tmp/commitstress" + Date.now() + "/";
try {
	require("wrench").rmdirSyncRecursive(path);
} catch(e) {}

var repo = gitteh.initRepository(path);

// Create an empty blob.
var blobRawObj = repo.createRawObject();
blobRawObj.type = "blob";
blobRawObj.data = new Buffer("Hello world!");
blobRawObj.save();

var createCommit = function(callback) {
	async.waterfall([
		function(callback) {
			async.parallel({
				commit: function(callback) { repo.createCommit(callback); },
				tree: function(callback) {
					repo.createTree(function(err, tree) {
						if(err) return callback(err);
						tree.addEntry(blobRawObj.id, (Math.random() * 10000) + "", 1, function(err) {
							if(err) return callback(err);
							
							tree.save(function(err) {
								if(err) return callback(err);
								callback(null, tree);
							});
							
						});
					});
				}
			}, callback);
		},
		
		function(objects, callback) {
			var commit = objects.commit;
			commit.setTree(objects.tree);

			commit.message = Math.random() * 10000;
			commit.author = commit.committer = {
				name: "Sam",
				email: "sam.c.day@gmail.com",
				time: new Date()
			};
			
			commit.save(function(err, res) {
				if(err) return callback(err);
				callback(null, commit);
			});
		}		
	], callback);
};

var fns = [];
var num = 10000;
for(var i = 0; i < num; i++) {
	fns.push(createCommit);
}

// Take the created commits, and pair them all up in a daisy chain so they can
// be walked through using git cli.
var chainCommits = function(commits) {
	var lastCommit = commits[0];
	
	for(var i = 1, len = commits.length; i < len; i++) {
		commits[i].addParent(lastCommit);
		commits[i].save();
		lastCommit = commits[i]; 
	}
};

var start = Date.now();
if(process.argv[1] == __filename) {
	async.parallel(fns, function(err, commits) {
		if(err) {
			console.log("ERROR!");
			throw err;
		}

		var time = Date.now() - start;
		console.log("Took " + time + "ms");
		console.log("Average " + (time/num) + "ms per commit/tree.");
		
		console.log("Chaining commits...");
		chainCommits(commits);
		var ref = repo.createOidReference("refs/heads/master", commits[commits.length-1].id);
	});
}
