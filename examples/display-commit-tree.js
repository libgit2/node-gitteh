/**
 * This example will load the Git repository for gitteh, and display the commit
 * tree of the latest revision. You have to have cloned the gitteh Git repo for
 * this to work, of course.
 */

var gitteh = require("gitteh"), 
	path = require("path"),
	fs = require("fs");

var repository = gitteh.openRepository(path.join(__dirname, "..", ".git"));
var headRef = repository.getReference("HEAD");
headRef = headRef.resolve();

var commit = repository.getCommit(headRef.target);

var displayTreeContents = function(treeId, tabs) {
	var tree = repository.getTree(treeId);
	
	var tabStr = ""; for(var i = 0; i < tabs; i++) tabStr += "  ";

	for(var i = 0, len = tree.entries.length; i < len; i++) {
		var entry = tree.entries[i];
		var line = tabStr ;
		line += entry.name;
		
		// 16384 == 40000 in octal (which is directory attribute in Git).
		if(entry.attributes == 16384) {
			line += "/";
			console.log(line);
			displayTreeContents(entry.id, tabs + 1);
		}
		else {
			//line += " - " + entry.id;
			console.log(line);
		}
	}
};

displayTreeContents(commit.tree, 1);
