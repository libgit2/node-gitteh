var gitteh = require("../lib/gitteh"),
	path = require("path");

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.tag("5eab735c859e9d808204532061a4316ea7d6b57a", function(err, tag) {
		if(err) return console.error(err);
		console.log(tag);
	});
});
