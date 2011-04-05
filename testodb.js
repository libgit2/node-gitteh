var gitteh = require("gitteh");
var path = require("path");
var fs = require("fs");

var odbDir = path.join(__dirname, "testpacked", ".git", "objects");

try {
var odb = gitteh.openODB(odbDir);
}
catch(e) {
	console.log(e.gitErrorStr);
	throw e;
}

console.log(odb.get("69906ba14f17bb026db62043e2359c97778ab5e7"));