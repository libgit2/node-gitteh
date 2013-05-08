var child_process = require("child_process");

child_process.exec("which cmake", function(err) {
	if(err && err.code != 0) {
		console.error("[ERROR] CMake is required for installation.")
		console.error("Please ensure CMake is installed to a standard location" +
			"in your system.");
		process.exit(1);
	}
});
