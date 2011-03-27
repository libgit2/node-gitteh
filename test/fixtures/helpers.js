var crypto = require("crypto"),
	path = require("path"),
	wrench = require("wrench"),
	gitteh = require("gitteh");

// Converts a decimal number to an octal number.
module.exports.toOctal = function(decimal) {
	return parseInt((decimal).toString(8));
};

// Converts octal back to decimal.
module.exports.fromOctal = function(octal) {
	return parseInt(octal+"", 8);
};

module.exports.getSHA1 = function(data) {
	var hash = crypto.createHash("sha1");
	hash.update(data);
	return hash.digest("hex");
};

module.exports.createTestRepo = function() {
	var repoPath = path.join(__dirname, "temprepos", new Date().getTime() + "" + Math.ceil(Math.random()*1000));
	var repo = gitteh.initRepository(repoPath);
	
	// Populate some dummy data.
	
	return repo;
};

module.exports.cleanupTemporaryRepo = function(repo) {
	try {
		wrench.rmdirSyncRecursive(repo.path);
	}
	catch(e) {}	
};