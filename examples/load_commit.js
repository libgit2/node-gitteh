var gitteh = require("../lib/gitteh");
var path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.commit("1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", function (err, commit) {
		meh = commit;
		console.log(commit);
	});
});
