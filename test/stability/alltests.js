// Aggregates all the async stability tests we have available, to make sure
// everything works as a whole.

var async = require("async");

var fns = [
	async.parallel.bind(null, require("./parent_traverse")),
	async.parallel.bind(null, require("./init_repo")),
	async.parallel.bind(null, require("./open_repo"))
];

async.parallel(fns, function(err) {
	if(err) {
		console.log("Uh oh!", err);
	}
	else {
		console.log("All done.");
	}
});
