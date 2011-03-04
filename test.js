var gitteh = require("./build/default/gitteh");

var startTime = Date.now();

//var repo = new gitteh.Repository(require("path").join(__dirname, "test", "fixtures", "gitrepo"));
var repo = new gitteh.Repository(require("path").join(__dirname, ".git"));

var tree = repo.getTree("8fb604142d77fca47ba5802fa7a29401282b3832");

console.log(tree.entries);
console.log(tree.entries.length);
console.log(tree.entries instanceof Array);
console.log(tree.blah.length);

//var tree = repo.getTree("8fb604142d77fca47ba5802fa7a29401282b3832");
/*
var commitId = require("fs").readFileSync(require("path").join(__dirname, ".git","refs","heads","master"), "utf8").trim().replace("\r", "").replace("\n", "");
var myCommit = repo.getCommit(commitId);
console.log(myCommit.tree.entries);
console.log(myCommit.tree.entries[0]);

for(var i = 0; i < myCommit.tree.entries.length; i++) {
	console.log(myCommit.tree.entries[i]);
}

/*
console.log(myCommit.tree.entries[0] === myCommit.tree.entries["README.md"]);
console.log(myCommit.tree.entries["README.md"]);
console.log("time", Date.now() - startTime);

console.log(repo.index);

console.log(myCommit.tree === repo.getTree("c100d82f652c29f6188c0bf4401d1d29aabc50da"));

myCommit = null;
*/



//console.log(repo.index);
/*console.log(repo.index.entries[1]);
for(var i = 0; i < repo.index.entries.length; i ++)
	console.log(repo.index.entries[i]);*/
//repo = null;


//profiler.gc();
//console.log("gc done.");

//console.log(repo.getTree("8fb604142d77fca47ba5802fa7a29401282b3832"));