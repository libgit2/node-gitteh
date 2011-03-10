var gitteh = require("./build/default/gitteh");
var path = require("path");

var repo = new gitteh.Repository(path.join(__dirname, ".git"));

var headRef = repo.getReference("HEAD");
headRef = headRef.resolve();

var commit = repo.getCommit(headRef.target, function(err, commit) {
	console.log(commit);
});
