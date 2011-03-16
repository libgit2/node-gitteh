// Step 1, open a repository, step 2, populate it with 1000 commits in an async
// manner. Step 3, iterate over all of these commits and mess with them.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

require("wrench").rmdirSyncRecursive("commitrepo");
var repo = gitteh.initRepository("commitrepo");

var createCommit = function(callback) {
	async.waterfall([
		function(callback) {
			async.parallel({
				commit: function(callback) { repo.createCommit(callback); },
				tree: function(callback) {
					repo.createTree();
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
			
			commit.save(callback.bind(null, commit));
		},
		
		function(commit, result, callback) {
			
		}
		
	], callback);
};

var fns = [];

for(var i = 0; i < 1000; i++) {
	fns.push(createCommit);
}

if(process.argv[1] == __filename) {
	async.parallel(fns, function() {
		console.log("All done!");
	});
}
