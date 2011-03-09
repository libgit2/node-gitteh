// This code will run numerous libgit2 tasks asynchronously, to ensure 
// thread-safety is working. For now that means that all gitteh bindings will
// use the gitLock mutex before they touch a git_* command, though this could 
// also be used as a reference point later if libgit2 itself becomes thread-safe

var gitteh = require("gitteh"),
	path = require("path"),
	async = require("async");

var repo = new gitteh.Repository(path.join(__dirname, ".git"));

var traverseCommits = function(commit, callback) {
	if(commit.parentCount == 0) callback(null, commit);
	for(var i = 0; i < commit.parentCount; i++) {
		commit.getParent(i, function(err, commit) {
			if(err) callback(err); traverseCommits(commit, callback);
		});
	}
};

var startCommitTraversal = function(callback) {
	traverseCommits(commit, callback);
};

var headRef = repo.getReference("HEAD");
headRef = headRef.resolve();

var commit = repo.getCommit(headRef.target);

var traversals = [];
for(var i = 0; i < 1; i++) {
	traversals.push(startCommitTraversal);
}

console.log("Starting from", commit);

async.parallel(traversals, function(err, rootCommit) {
	if(err) {
		console.log("FAILED.");
		console.log(err);
	}
	else {
		console.log("All done!");
		console.log(rootCommit);
	}
});