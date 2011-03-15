var gitteh = require("./build/default/gitteh");
var path = require("path");
var async = require("async");
var profiler = require("profiler");

var repo = gitteh.openRepository(path.join(__dirname, ".git"));

var commit = repo.getCommit("f02b077372ebc200dca09be8e7b9732300646eb2");
commit.getTree(function() {
console.log(arguments);
});
repo.getTree("8988db760a1b7885d146d6610469304207c03582");
commit.getTree();

return;

var num = 50;

var commit = repo.getCommit("f02b077372ebc200dca09be8e7b9732300646eb2");
/*
for(var i = 0; i < 50; i++) {
	(function(i) {
		commit.getParent(0, function(err, commit) {
			console.log("worker " + i + ": err - " + err + " ID - " + commit.id);
		});
	})(i);
}
return;*/

var commitsTraversed = 0;
var i = 0;
var traverseParents = function(commit, workerNum, callback) {
	var found = false;
	var start = Date.now();
	
	var traverser = function(commit) {
		if(!commit.parentCount && !found) {
			console.log("worker " + workerNum + " done in " + (Date.now() - start) + "!");
			//console.log(commit);
			found = true;
			callback(null, commit);
			return;
		}

		//console.log(workerNum + " working...");
		//console.log(commit);
	
		//console.log(commit.id.toUpperCase());

		for(var i = 0; i < commit.parentCount; i++) {
			//traverseParents(commit.getParent(i));
			//(function(i) { process.nextTick(function() {
			//(function(i) { setTimeout(function() {
				commit.getParent(i, function(err, commit) {
				//console.log(arguments);
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
	fns.push(traverseParents.bind(null, commit, i + 1));
	//traverseParents(commit);

var numRuns = 1;
var timer = setInterval(function() {
	if(numRuns == 2) {
		clearInterval(timer);
		return;
	}

	var jobNum = numRuns++;
	var start = Date.now();
	async.parallel(fns, function(err) {
		//console.log("All done!");
		
		var time = Date.now() - start;
		profiler.gc();
		//console.log("Took " + time + "ms");
		//console.log("Avg traverse: " + (time/num) +"ms");
	});
}, 100);

/*
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
})();*/
/*setTimeout(function() {
	console.log("All done.");
}, 2000);*/