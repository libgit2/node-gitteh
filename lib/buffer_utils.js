// Some quick extensions to Buffer. Primarily adding methods to read/write data in network byte ordering (big endian).
module.exports = {
	getInt16: function(buffer, offset) {
		return buffer[offset] << 8 | buffer[offset + 1];
	},
	
	getInt32: function(buffer, offset) {
		var val = ((buffer[offset] & 0x7F) << 24) | (buffer[offset + 1] << 16) | (buffer[offset + 2] << 8) | (buffer[offset + 3]);
		
		// If the most significant bit is 1, then we add 2147483648 to the number.
		// This occurs because whenever we use bitwise operations, Javascript will treat the number as a 32bit SIGNED integer, which is bad as the MSB
		// (most significant bit) indicates sign (+/-) of number. So why do we add 2147483648 to the number manually if the MSB is set?
		// Simple, because 2147483648 == 10000000000000000000000000000000, which is the most significant bit :)
		if(buffer[offset] & 0x80) val += 2147483648;
		return val;
	},
	
	setInt16: function(buffer, offset, val) {
		val = val & 0xFFFF;
		buffer[offset] = (val & 0xFF00) >> 8;
		buffer[offset + 1] = val;
	},
	
	setInt32: function(buffer, offset, val) {
		buffer[offset] = val >> 24 & 0xFF;
		buffer[offset + 1] = val >> 16 & 0xFF;
		buffer[offset + 2] = val >> 8 & 0xFF;
		buffer[offset + 3] = val & 0xFF;
	}
};
