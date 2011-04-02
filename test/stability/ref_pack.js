var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs"),
	helpers = require("../fixtures/helpers");

var num = 100;

process.setMaxListeners(num);
var repos = [];
console.log("Preparing test repos.");
for(var i = 0; i < num; i++) {
	repos[i] = helpers.createTestRepo("refstress"+i);
}
console.log("Done. Starting test.");

var packRefsWithOpeningRefs = function(i, callback) {
	var packDone = false;
	var renameDone = false;

	var repo = repos[i];

	async.parallel({		
		ref: function(callback) {
			repo.getReference("refs/heads/master", function(err, ref) {
				if(err) return callback(err);
				
				ref.rename("refs/heads/renamed");
				if(repo.getReference("refs/heads/renamed") !== ref) {
					return callback(new Error("Failed."));
				}

				renameDone = true;
				callback(null, ref);
			});
		},
		
		deleteref: function(callback) {
			var ref = repo.getReference("refs/heads/test");
			ref.delete(function(err, result) {
				if(err) return callback(err);
				
				if(!result) {
					return callback(new Error("failed."));
				}
				
				repo.getReference("refs/heads/test", function(err, ref) {
					if(!err || ref) return callback(new Error("Failed."));

					if(err.gitError != gitteh.error.GIT_ENOTFOUND) {
						return callback(new Error("Wrong error back from gitteh."));
					}
					
					callback(null, true);
				});
			});
		},
		
		pack: function(callback) {
			repo.packReferences(callback);
		}
	}, callback);
};

var fns = module.exports = [];
for(var i = 0; i < num; i++) {
	(function(i) {
		fns.push(function(callback) { packRefsWithOpeningRefs(i, callback); });
	})(i);
}

if(process.argv[1] == __filename) {
	var start = Date.now();
	async.parallel(fns, function(err, results) {
		if(err) throw err;

		console.log("All done!");
		console.log("Took " + (Date.now()-start) + "ms.");
	});
}
