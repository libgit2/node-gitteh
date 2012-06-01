var gitteh = require("../lib/gitteh");
var path = require("path");
var temp = require("temp");

var repoPath = temp.mkdirSync();
var repo = "http://github.com/libgit2/node-gitteh.git";

console.log("Cloning " + repo + " into " + repoPath);
gitteh.clone(repo, repoPath, function(err, repo) {
	console.log(arguments);
});
