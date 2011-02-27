var path = require("path");

var blah = require("./build/default/libgit2_bindings");

for(var i = 0; i < 1; i++) {
	var myRepo = new blah.Repository(path.join(__dirname, ".git"));
	/*console.log(myRepo);
	var odb = myRepo.getObjectDatabase();
	console.log(odb);
	var obj = odb.read("8a916d5fbce49f5780668a1ee780e0ef2e89360f");
	console.log(obj);
	console.log(obj.type);
	console.log(obj.type);
	var data = obj.data;
	console.log(data);
	console.log(data.toString());*/

	var commit = myRepo.getCommit("8a916d5fbce49f5780668a1ee780e0ef2e89360f");
	console.log(commit);
	console.log(commit.message);
	console.log(commit.messageShort);
	console.log(commit.time);
	console.log(commit.author);
	console.log(commit.committer);
	console.log(commit.id);
	console.log(commit == myRepo.getCommit("8a916d5fbce49f5780668a1ee780e0ef2e89360f"))

/*
	console.log(commit.tree == commit.tree);
	var tree = commit.tree;
	console.log(tree);
	console.log(tree.id);
	console.log(tree.length);
	
	var entry = tree[0];
	console.log(entry);
	console.log(entry.name);
	
	//delete myRepo;
	//myRepo = null;*/

	commit = null;
	myRepo = null;
	
	require("profiler").gc();
	
	
	console.log(".");
}
