var gitteh = require("../lib/gitteh"),
	path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.blob("aa4306ebb3e97b8ec76136feab1bb5fcd096b28a", function(err, blob) {
		if(err) return console.error(err);
		console.log(blob);
		console.log(blob.data.toString("UTF-8"));
	});
});
