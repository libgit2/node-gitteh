var gitteh = require("./build/default/gitteh");
var path = require("path");
var async = require("async");

var repo = new gitteh.Repository(path.join(__dirname, ".git"));

var num = 50;

//var headRef = repo.getReference("HEAD");
//headRef = headRef.resolve();

var commit = repo.getCommit("f02b077372ebc200dca09be8e7b9732300646eb2");
/*
for(var i = 0; i < 10000; i++) {
	repo.getCommit(headRef.target, function() {});
	repo.createCommit(function() {});
	repo.createTree(function() {});
	repo.getTree(headRef.target, function() {});
}
*/
var commitsTraversed = 0;
var i = 0;
var traverseParents = function(commit, workerNum, callback) {
	var found = false;
	var start = Date.now();
	
	var traverser = function(commit) {
		if(!commit.parentCount && !found) {
			console.log("worker " + workerNum + " done in " + (Date.now() - start) + "!");
			found = true;
			callback(null, commit);
			return;
		}
		
		//console.log(workerNum + " working...");
		//console.log(commit);
	
		for(var i = 0; i < commit.parentCount; i++) {
			//traverseParents(commit.getParent(i));
			//(function(i) { process.nextTick(function() {
			//(function(i) { setTimeout(function() {
				commit.getParent(i, function(err, commit) {
					if(err) {
						console.log("OH FUCK!", err);
						process.exit(-1);
					}
					traverser(commit, callback);
				});
			//}, 1);})(i);
			//}); })(i);
		}
	};
	
	traverser(commit);
};

var fns = [];
for(var i = 0; i < num; i++)
	fns.push(traverseParents.bind(null, commit, i));
	//traverseParents(commit);

var start = Date.now();
async.parallel(fns, function() {
	console.log("All done!");
	var time = Date.now() - start;
	console.log("Took " + time + "ms");
	console.log("Avg traverse: " + (time/num) +"ms");
});

(function() {
	var num = 10;

	var commit = repo.getCommit("f02b077372ebc200dca09be8e7b9732300646eb2");

	var traverseParents = function(commit, workerNum, callback) {
		var found = false;
		
		var traverser = function(commit) {
			if(!commit.parentCount && !found) {
				found = true;
				callback(null, commit);
				return;
			}
		
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
})();
/*setTimeout(function() {
	console.log("All done.");
}, 2000);*/