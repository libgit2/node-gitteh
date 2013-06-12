var async = require("async"),
	child_process = require("child_process"),
	spawn = child_process.spawn,
	path = require("path"),
	request = require('request'),
	fs = require('fs'),
	tar = require('tar'),
	zlib = require('zlib');

var isWin = !!process.platform.match(/^win/);

function passthru() {
	var args = Array.prototype.slice.call(arguments);
	var cb = args.splice(-1)[0];
	var cmd = args.splice(0, 1)[0];
	var opts = {};
	if(typeof(args.slice(-1)[0]) === "object") {
		opts = args.splice(-1)[0];
	}
	var child = spawn(cmd, args, opts);

	child.stdout.pipe(process.stdout);
	child.stderr.pipe(process.stderr);
	child.on("exit", cb);
}

function shpassthru() {
	var cmd = 
	passthru.apply(null, ["/bin/sh", "-c"].concat(Array.prototype.slice.call(arguments)));
}

function envpassthru() {
	passthru.apply(null, ["/usr/bin/env"].concat(Array.prototype.slice.call(arguments)));
}

var libgit2Dir = path.join(__dirname, "deps/libgit2");
var buildDir = path.join(libgit2Dir, "build");
async.series([
	function(cb) {
		console.log("[gitteh] Downloading libgit2 dependency.");
		if (fs.existsSync(path.join(__dirname, '.git'))) {
			console.log("[gitteh] ...using git");
			passthru("git", "submodule", "update", "--init", cb);
		} else {
			console.log("[gitteh] ...from GitHub");
			var libgit2Version = "v0.17.0";
			var url = "https://github.com/libgit2/libgit2/tarball/" + libgit2Version;
			request({url: url})
				.pipe(zlib.createUnzip())
				.pipe(tar.Extract({
					path: libgit2Dir,
					strip: true
				})).on('end', cb);
		}
	},
	function(cb) {
		console.log("[gitteh] Building libgit2 dependency.");
		if (isWin && !fs.existsSync(buildDir)) {
			passthru("mkdir", buildDir, cb);
		} else {
			passthru("mkdir", "-p", buildDir, cb);
		}
	},
	function(cb) {
		passthru("cmake", "-DBUILD_SHARED_LIBS=OFF", "_DSTDCALL=OFF", "-DTHREADSAFE=ON", "-DBUILD_CLAR=OFF", "..", {
			cwd: buildDir
		}, cb);
	},
	function(cb) {
		passthru("cmake", "--build", ".", {
			cwd: buildDir
		}, cb);
	},
	function(cb) {
		console.log("[gitteh] Building native module.");
		shpassthru("./node_modules/.bin/node-gyp configure", cb);
	},
	function(cb) {
		shpassthru("./node_modules/.bin/node-gyp build", cb);
	},
	function(cb) {
		shpassthru("./node_modules/.bin/coffee -o lib/ -c src/", cb);
	}
], function(err) {
	if(err) process.exit(err);
});
