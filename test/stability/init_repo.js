var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

var timestamp = Date.now();
var path = "/tmp/" + timestamp;
fs.mkdirSync(path, "755");

process.on("exit", function() {
	require("wrench").rmdirSyncRecursive(path);
});

var fns = [];
for(var i = 0; i < 100; i++) {
	fns.push(function(i, callback) {
		gitteh.initRepository(path + "/" + i, true, callback);
	}.bind(i));
}

if(process.argv[1] == __filename) {
	async.parallel(fns, function() {
		console.log("All done!");
	});
}
