var gitteh = require("./build/default/gitteh");
var path = require("path");
var profiler = require("profiler");
var async = require("async");

var repo = gitteh.openRepository(path.join(__dirname, ".git"));

var num = 1;

var commit = repo.getCommit("f02b077372ebc200dca09be8e7b9732300646eb2");

var traverseParents = function(commit, workerNum, callback) {
	var found = false;
	
	var traverser = function(commit) {
		if(!commit.parentCount && !found) {
			found = true;
			callback(null, commit);
			return;
		}
		
		console.log(commit.id);
	
		for(var i = 0; i < commit.parentCount; i++) {
			traverser(commit.getParent(i));
		}
	};
	
	traverser(commit);
};

var fns = [];
for(var i = 0; i < num; i++)
	fns.push(traverseParents.bind(null, commit, i));

var start = Date.now();
async.parallel(fns, function() {
	console.log("All done!");
	var time = Date.now() - start;
	console.log("Took " + time + "ms");
	console.log("Avg traverse: " + (time/num) +"ms");
});