var gitteh = require("../lib/gitteh");
var path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.remote("test", function(err, remote) {
		console.log(remote);

		remote.connect("fetch", function(err) {
			if(err) return console.error(err);
			console.log(remote.connected);
		});
	});
});
