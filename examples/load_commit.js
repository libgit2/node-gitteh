var gitteh = require("../lib/gitteh");
var path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.commit("8a916d5fbce49f5780668a1ee780e0ef2e89360f", function (err, commit) {
		meh = commit;
		console.log(commit);
	});
});
