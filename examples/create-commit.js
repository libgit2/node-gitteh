var gitteh = require("gitteh"), 
	path = require("path"),
	fs = require("fs");

var repository = gitteh.openRepository(path.join(__dirname, "..", ".git"));

var newCommit = repository.createCommit();

// A commit starts off in something of in an "identity" state. Pretty much 
// everything is null. We have to populate the data before a save can happen.

newCommit.author = newCommit.committer = {
	name: "Sam Day",
	email: "sam.c.day@gmail.com",
	time: new Date()
};

newCommit.message = "A demo commit.";

// A commit needs to have a Tree associated with it. This contains the actual 
// files that make up the commit.
var myTree = repository.createTree();

// Each tree entry references another git object. Generally a tree entry will
// either reference a "blob", which is an actual file, or it will reference 
// another tree, which is a subdirectory.

// In this case, we'll just add a single blob to the tree.
var myBlob = repository.createRawObject();
myBlob.data = new Buffer("Hello world!");
myBlob.type = "blob";
myBlob.save();
console.log("Created a test BLOB. ID = " + myBlob.id);

// At this point, our blob now has an identity, we can add it to the tree now.
// 33188 is the file attributes. It's the decimal equiv of 100644 in octal.
myTree.addEntry(myBlob.id, "test", 33188);

// We can now save the tree.
myTree.save();

console.log("Created a test TREE. ID = " + myTree.id);

// At this point, our tree now exists in the Git object database, and it has
// an identity. We can add it to the commit now.

// You can set the tree of a commit either with a Tree object, or with a string
// id of a pre-existing tree.
newCommit.setTree(myTree);

// We're ready to finally create our commit now. How exciting!
newCommit.save();

console.log("Created a test COMMIT. ID = " + newCommit.id);

// Note that you won't see this commit in a git log. This is because it's not 
// part of the Directed Acyclic Graph (DAG). That is to say, there's no way to
// follow references to this commit via any of the refs (HEAD/refs/heads/master,
// etc). 

// You can view the commit directly using git by executing a command like this:
// git log <commit id>
// Ex: git log ed7c24a4f6909465eb478e9a969c4d65fc0afcd7