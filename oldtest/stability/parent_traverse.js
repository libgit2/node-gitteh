// This stress test will open a repository and traverse all commits, doing this
// many times simultaneously. This will ensure there's no deadlocks or other fun
// threading issues.
var gitteh = require("../../build/default/gitteh"),
	async = require("async"),
	path = require("path");

var traverseParents = function(commit, callback) {
	var found = false;

	var traverser = function(commit) {
		if(!commit.parentCount && !found) {
			found = true;
			callback(null, commit);
			return;
		}

		for(var i = 0; i < commit.parentCount; i++) {
			commit.getParent(i, function(err, commit) {
				if(err) {
					callback(err);
					return;
				}
				else {
					traverser(commit);
				}
			});
		}
	};

	traverser(commit);
};

var fns = module.exports = [];

var repo = gitteh.openRepository(path.join(__dirname, "..", "..", ".git"));
var headRef = repo.getReference("HEAD");
headRef = headRef.resolve();
var commit = repo.getCommit(headRef.target);

for(var i = 0; i < 1; i++) {
	fns.push(traverseParents.bind(null, commit));
}

if(process.argv[1] == __filename) {
	async.parallel(fns, function() {
		console.log("All done!");
	});
}