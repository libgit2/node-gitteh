// This test makes sure we can retrieve the same resource both async and sync
// simultaneously.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	profiler = require("profiler");
	
var repo = gitteh.openRepository(path.join(__dirname, "..", "..", ".git"));


console.log(repo.createCommit(function() {
console.log(arguments);
}));
return;

var headRef = repo.getReference("HEAD");
headRef = headRef.resolve();

var syncCommit = null;

var num = 1;

var cleanupGoAgain = function() {
	syncCommit = null;
	profiler.gc();
	
	profiler.gc();
	runCheck(++num);
};

var runCheck = function(job) {
	if(job > 10000) {
		console.log("All done.");
		profiler.gc();
		return;
	}

	repo.getCommit(headRef.target, function(err, commit) {
		if(syncCommit !== commit) {
			console.log(arguments);
			console.log("FAILED.");
			process.exit(-1);
		}
	
		console.log("Iteration #" + job + " was successful.");
		//console.log(commit);
		delete commit;
		commit = null;
		cleanupGoAgain();
	});
	syncCommit = repo.getCommit(headRef.target);
	profiler.gc();
};

runCheck(num);