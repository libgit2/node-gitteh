var gitteh = require("gitteh"),
	path = require("path");
	
var repo = gitteh.openRepository(path.join(__dirname, ".git"));
console.log(repo.getCommit("423a767b98c878beb792b04408683917e639867c"));