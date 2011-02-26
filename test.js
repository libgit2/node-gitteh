var path = require("path");

var blah = require("./build/default/libgit2_bindings");

for(var i = 0; i < 1; i++) {
	var myRepo = new blah.Repository(path.join(__dirname, ".git"));
	console.log(myRepo);
	var odb = myRepo.getObjectDatabase();
	console.log(odb);
	var obj = odb.read("8a916d5fbce49f5780668a1ee780e0ef2e89360f");
	console.log(obj);
	console.log(obj.type);
	var data = obj.data;
	console.log(data);
	console.log(data.toString());
	delete myRepo;
	myRepo = null;
}
