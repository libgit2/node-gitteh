var gitteh = require("../lib/gitteh");
var path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	exports.repo = repo;

	repo.remote("test", function(err, remote) {
		exports.remote = remote;

		remote.connect("fetch", function(err) {
			if(err) return console.error(err);
			console.log(remote);
			console.log(remote.connected);
			console.log(remote.refs);
		});
	});
});
