var crypto = require("crypto");

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