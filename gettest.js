var gitteh = require("gitteh"),
	path = require("path"),
	fs = require("fs");

var repository = gitteh.openRepository(path.join(__dirname, ".git"));

var ref = repository.getReference("refs/heads/master");
var ref = ref.resolve();

var commit = repository.getCommit(ref.target);
console.log(commit);

var commit = repository.getCommit(ref.target);
console.log(commit);

Object.prototype.clone = function() {
  var newObj = (this instanceof Array) ? [] : {};
  for (i in this) {
    if (i == 'clone') continue;
    if (this[i] && typeof this[i] == "object") {
      newObj[i] = this[i].clone();
    } else newObj[i] = this[i]
  } return newObj;
};

//var newCommitData = commit.clone();
var newCommit = repository.createCommit({
	message: "Hello.",
	author: {
		name: "Sam",
		email: "sam@day.com",
		time: new Date()
	},
	committer: {
		name: "Sam",
		email: "sam@day.com",
		time: new Date()
	},
	tree: commit.tree
});
console.log(newCommit);