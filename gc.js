var gitteh = require("./build/default/gitteh");
var path = require("path");

var repo = gitteh.openRepository(path.join(__dirname, ".git"));

var fs = require("fs");
var commits = fs.readFileSync("commits.txt", "utf8").split("\n");
console.log(commits.length);

var totalRead = 0;
for(var j = 0; j < 10000; j++) { 
	for(var i = 0; i < commits.length; i++) {
		if(commits[i]) {
			repo.getCommit(commits[i], function(err, commit) {
				if(err) console.log(err);
				else totalRead ++;
				//console.log(commit.id); 
			});
		}
	}
}

setInterval(function() {
console.log(totalRead);
}, 1000);