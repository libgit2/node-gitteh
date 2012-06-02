var gitteh = require("../lib/gitteh");
var path = require("path");
var temp = require("temp");

var repoPath = temp.mkdirSync();
var repo = "http://github.com/libgit2/node-gitteh.git";

console.log("Cloning " + repo + " into " + repoPath);
var clone = gitteh.clone(repo, repoPath);

clone.on("status", function(bytes, total, done) {
	var updateString = "Transferred " + bytes + " bytes.";
	if(done > 0) updateString += " (" + done + "/" + total + " objects )"
	process.stdout.write("\r" + updateString);
});

clone.on("complete", function(repo) {
	console.log("\n... Complete!");
	console.log(repo);
});

clone.on("error", function(err) {
	console.log("\n");
	console.error(err);
});