var gitteh = require("./build/default/gitteh"),
	path = require("path");

var repo = new gitteh.Repository(path.join(__dirname, ".git"));

for(var i = 0; i < 1000; i++) {
	var start = Date.now();
	var commit = repo.getCommit("f5ec519561274c78bfeabe474753cfe162cb84b2", function(error, commit) {
		//console.log("got the commit.");
		//console.log(error);
		//console.log(commit);
		
		//console.log(Date.now() - start);
	});
}

console.log("first!", repo.getCommit("f5ec519561274c78bfeabe474753cfe162cb84b2"));

setTimeout(function() {
console.log("second!", repo.getCommit("f5ec519561274c78bfeabe474753cfe162cb84b2"));
}, 10);