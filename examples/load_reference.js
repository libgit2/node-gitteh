var gitteh = require("../lib/gitteh"),
	path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.ref("HEAD", true, function(err, ref) { 
		console.log(ref);
	});
});
