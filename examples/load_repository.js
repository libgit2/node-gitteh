var gitteh = require("../lib/gitteh");
var path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.remotes.push("hehe");
	console.log(repo);
});
