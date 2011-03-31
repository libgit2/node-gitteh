var gitteh = require("gitteh"),
	path = require("path");

var repo = gitteh.openRepository(path.join(__dirname, ".git"));

for(var i = 0; i < 10; i++) {
var commit = repo.getCommit("5108bce15012edcceb48f9ff25f519e5eb5c15ae", function(err, commit) {
	console.log("commitargs", arguments);gc();
});

console.log("commit", repo.getCommit("5108bce15012edcceb48f9ff25f519e5eb5c15ae"));
gc();
}

/*var commit = repo.getCommit("5108bce15012edcceb48f9ff25f519e5eb5c15ae", function(err, commit) {
	console.log("commit", arguments);
});*/