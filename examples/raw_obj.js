var gitteh = require("gitteh"), 
	path = require("path"),
	fs = require("fs");

var repository = new gitteh.Repository(path.join(__dirname, "..", ".git"));

var headCommit = fs.readFileSync(path.join(
		__dirname, "..", ".git", "refs", "heads", "master"), "utf8");

var commit = repository.getCommit(headCommit);

// We can open any object in Git in raw mode, which will give us access to the 
// entirety of an entity stored in Git.
var tree = commit.getTree();

var rawObj = repository.getRawObject(commit.id);
console.log("COMMIT RAW DATA");
console.log("===============");
console.log(rawObj.data.toString());

console.log("\n\n\nTREE RAW DATA");
console.log("=============");
rawObj = repository.getRawObject(tree.id);
console.log(rawObj.data.toString());

rawObj = repository.getRawObject(tree.getEntry(0).id);
console.log("\n\n\nBLOB RAW DATA");
console.log("=============");
console.log(rawObj.data.toString());

//You can write data in raw mode if you want (read: if you're a masochist).
//But neither Gitteh nor libgit2 will hold your hand if you fuck something up.