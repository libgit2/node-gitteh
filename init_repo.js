var gitteh = require("./build/default/gitteh");
var path = require("path");
var async = require("async");
var fs = require("fs");

var timestamp = Date.now();
var path = "/tmp/" + timestamp;
fs.mkdirSync(path, "755");

for(var i = 0; i < 100; i++) {
	gitteh.initRepository(path + "/" + i, true, function() {
		console.log("repo " + i + " created.");
	});
}

