var gitteh = require("./lib/gitteh");
var path = require("path");

try {
	console.log(gitteh.attemptPack(path.join(__dirname, "pack-11e22b43bbbd6e3ac9bf9b3a233229d99696a1d6.pack")));	
	//console.log(gitteh.attemptPack(path.join(__dirname, "testpack.pack")));
}
catch(e) {
	console.log(e);
	throw e;
}