// Converts a decimal number to an octal number.
module.exports.toOctal = function(decimal) {
	return parseInt((decimal).toString(8));
};

// Converts octal back to decimal.
module.exports.fromOctal = function(octal) {
	return parseInt(octal+"", 8);
};