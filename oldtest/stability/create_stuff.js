// Open a new repository, and create a shiteload of random objects.
// TODO: actually save them once we have save asynchronous.

var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

var timestamp = Date.now();
var path = "/tmp/createstuff" + timestamp;
fs.mkdirSync(path, "755");

process.on("exit", function() {
	require("wrench").rmdirSyncRecursive(path);
});

var repo = gitteh.initRepository(path, true);
var fns = [];

for(var i = 0; i < 100; i++) {
	fns.push(function(callback) {
		async.parallel([
			function(callback) {
				repo.createCommit(function() {
					console.log(arguments);
				});
			},
			/*
			function(callback) {
				repo.createTree(callback);
			},
			
			function(callback) {
				repo.createTag(callback);
			}*/
		], callback);
	});
}

if(process.argv[1] == __filename) {
	async.parallel(fns, function() {
		console.log("All done!");
	});
}
