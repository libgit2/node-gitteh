var gitteh = require("../../build/default/gitteh"),
	path = require("path"),
	async = require("async"),
	fs = require("fs");

var fns = module.exports = [];
for(var i = 0; i < 100; i++) {
	fns.push(function(i, callback) {
		gitteh.openRepository(path.join(__dirname, "..", "..", ".git"), callback);
	}.bind(null, i));
}

if(process.argv[1] == __filename) {
	async.parallel(fns, function() {
		console.log("All done!");
	});
}
