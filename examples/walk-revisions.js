var gitteh = require("gitteh"),
	path = require("path"),
	fs = require("fs");

// Note that we're manually grabbing the master ref from the repo. Gitteh will
// eventually feature a ref-management api.
var headCommit = fs.readFileSync(path.join(
		__dirname, "..", ".git", "refs", "heads", "master"), "utf8");

// Load up the node-gitteh repository. This will only work if you cloned the 
// repo of course.
// You can point this to anywhere that is housing a git repo, even a bare one.
// You have to point it to the GIT directory though, so if you're working with
// a repo that has a working copy checked out, you need to point it to the .git
// folder.
var repository = new gitteh.Repository(path.join(__dirname, "..", ".git"));

// Let's create a revision walker and traverse the entire commit history.
var walker = repository.createWalker();

// Let's push the head commit onto the revision walker and start walkin'!

// This will sort our commits by time. They can also be sorted "topologically",
// which will prioritize parents before children. Also SORT_REVERSE can be
// added to sorting, which will reverse all sorting options chosen.

// This will start from the most recent commit, and go back in time.
// Note that you have to set sorting BEFORE you push a commit to traverse from.
walker.sort(gitteh.SORT_TIME);
walker.push(repository.getCommit(headCommit));

// This output basically mimicks a basic `git log` command.
var commit;
while(commit = walker.next()) {
	console.log("commit " + commit.id);

	console.log("Author: " + commit.author.name + " <" + 
			commit.author.email + ">");
	
	if((commit.committer.name != commit.author.name) || 
			(commit.committer.email != commit.author.email)) {
		console.log("Committer: " + commit.committer.name + " <" + 
				commit.committer.email + ">");
	}

	console.log("Date: " + commit.committer.time.toUTCString());
	
	console.log("\n    " + commit.message + "\n");
}
