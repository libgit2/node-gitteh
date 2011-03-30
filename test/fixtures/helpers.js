var crypto = require("crypto"),
	path = require("path"),
	wrench = require("./wrench"),
	gitteh = require("gitteh"),
	fs = require("fs");

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

module.exports.createTestRepo = function(prefix) {
	var destination = path.join("/tmp", "gitteh" + prefix + "test" + module.exports.getSHA1(Date.now() + Math.round(Math.random()*10000)))

	fs.mkdirSync(destination, "777");
	// Duplicate temp repo template.
	wrench.copyDirSyncRecursive(path.join(__dirname, "temp_repo_template"), destination);
	fs.renameSync(path.join(destination, "dotgit"), path.join(destination, ".git"));
	
	process.on("exit", function() {
		try {
			wrench.rmdirSyncRecursive(destination);
		}
		catch(e) {};
	});
	
	var repo = gitteh.openRepository2({
		gitDirectory: path.join(destination, ".git"),
		workTree: destination
	});
	
	// For convenience, these are the git objects that reside in the temp repo template.
	repo.TEST_COMMIT = "4a1e4c8d3e2a8ea94a049768a8b558f247b5f105";
	repo.TEST_TREE = "5302e718dddce0c57533e4a2d6cce175aa37653c";
	repo.TEST_BLOB = "05d2c9c26d23c6ad9f8dde127e2cab9256be6ab2";
	repo.TEST_TAG = "39da184ab61a20c80d1cafecd7d05352614cb837";
	repo.HEAD_COMMIT = "85b851a1a139400ecfe56cb3b656938d0eaff519";

	return repo;
};
