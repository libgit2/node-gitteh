var gitteh = require("gitteh"), 
	path = require("path"),
	fs = require("fs");

var repository = new gitteh.Repository(path.join(__dirname, "..", ".git"));

var headCommit = fs.readFileSync(path.join(
		__dirname, "..", ".git", "refs", "heads", "master"), "utf8");

var commit = repository.getCommit(headCommit);

var displayTreeContents = function(tree, tabs) {
	var tabStr = ""; for(var i = 0; i < tabs; i++) tabStr += "  ";
	
	for(var i = 0, len = tree.entryCount; i < len; i++) {
		var entry = tree.getEntry(i);
		var line = tabStr ;
		line += entry.filename;
		
		// 16384 == 40000 in octal (which is directory attribute in Git).
		if(entry.attributes == 16384) {
			line += "/";
			console.log(line);
			displayTreeContents(repository.getTree(entry.id), tabs + 1);
		}
		else {
			//line += " - " + entry.id;
			console.log(line);
		}
	}
};

displayTreeContents(commit.getTree(), 1);
