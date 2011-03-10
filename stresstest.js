// This code will run numerous libgit2 tasks asynchronously, to ensure 
// thread-safety is working. For now that means that all gitteh bindings will
// use the gitLock mutex before they touch a git_* command, though this could 
// also be used as a reference point later if libgit2 itself becomes thread-safe

var gitteh = require("gitteh"),
	path = require("path"),
	async = require("async");

var repo = new gitteh.Repository(path.join(__dirname, ".git"));
var headRef = repo.getReference("HEAD");
headRef = headRef.resolve();
/*
for(var i = 0; i < 10000; i++) {
	repo.getCommit(headRef.target, function(err) { if(err) console.log(err); });
}

setTimeout(function() {
}, 5000);
*/

var traverseCommits = function(commit, callback) {
	if(commit.parentCount == 0) {
		console.log("done.");
		callback(null, commit);
	}

console.log(commit);
	for(var i = 0; i < commit.parentCount; i++) {
		commit.getParent(i, function(err, commit) {
			if(err) callback(err); traverseCommits(commit, callback);
		});
	}
};

var startCommitTraversal = function(callback) {
	traverseCommits(commit, callback);
};


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

setTimeout(function() {
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
console.log("2secs.");
}, 2000);
/*
var fs = require("fs");
var commits = fs.readFileSync("commits.txt", "utf8").split("\n");
console.log(commits);

for(var i = 0; i < commits.length; i++) {
	if(commits[i]) {
		repo.getCommit(commits[i], function(err) {
			if(err) console.log(err);
			console.log(arguments[1]);
		});
	}
}

setTimeout(function() {
console.log("2secs.");
}, 2000);*/