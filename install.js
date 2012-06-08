var async = require("async"),
	child_process = require("child_process"),
	spawn = child_process.spawn,
	path = require("path");

function passthru() {
	var args = Array.prototype.slice.call(arguments);
	var cb = args.splice(-1)[0];
	var opts = {};
	if(typeof(args.slice(-1)[0]) === "object") {
		opts = args.splice(-1)[0];
	}
	console.log(args, opts);
	var child = spawn("/usr/bin/env", args, opts);

	child.stdout.pipe(process.stdout);
	child.stderr.pipe(process.stderr);
	child.on("exit", cb);
}

console.log("[gitteh] Installing libgit2 dependency.");

var buildDir = path.join(__dirname, "deps/libgit2/build");
async.series([
	function(cb) {
		passthru("git", "submodule", "init", cb);
	},
	function(cb) {
		passthru("git", "submodule", "update", cb);
	},
	function(cb) {
		passthru("mkdir", "-p", buildDir, cb);
	},
	function(cb) {
		passthru("cmake", "-DTHREADSAFE=1", "-DBUILD_CLAR=0", "..", {
			cwd: buildDir
		}, cb);
	},
	function(cb) {
		passthru("cmake", "--build", ".", {
			cwd: buildDir
		}, cb);
	}
]);
