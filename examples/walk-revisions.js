/**
 * This example will open the Gitteh Git repo and walk the entire revision
 * history, displaying a screen output fairly similar to what you'd get when you
 * run `git log`.
 */
var gitteh = require("gitteh"),
	path = require("path"),
	fs = require("fs");

var startTime = Date.now();

// Load up the node-gitteh repository. This will only work if you cloned the 
// repo of course.
// You can point this to anywhere that is housing a git repo, even a bare one.
// You have to point it to the GIT directory though, so if you're working with
// a repo that has a working copy checked out, you need to point it to the .git
// folder.
var repository = gitteh.openRepository(path.join(__dirname, "..", ".git"));

// First step is to grab the HEAD commit. We use the ref management features of
// gitteh to achieve this.
var headRef = repository.getReference("HEAD");

// Just in case the reference is pointing to another reference (symbolic link),
// we "resolve" the reference to a direct reference (one that points to an OID).
// If the ref being pointed to by HEAD is already direct, then resolve does 
// nothing but return the same reference.
headRef = headRef.resolve();

// Let's create a revision walker and traverse the entire commit history.
var walker = repository.createWalker();

// Let's push the head commit onto the revision walker and start walkin'!

// This will sort our commits by time. They can also be sorted "topologically",
// which will prioritize parents before children. Also SORT_REVERSE can be
// added to sorting, which will reverse all sorting options chosen.

// This will start from the most recent commit, and go back in time.
// Note that you have to set sorting BEFORE you push a commit to traverse from.
walker.sort(gitteh.GIT_SORT_TIME);
walker.push(headRef.target);

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

console.log((Date.now() - startTime) + "ms");