var async    = require("async")
    , path      = require('path')
    , fs          = require('fs')
    , request = require('request')
    , zlib        = require('zlib')
    , tar         = require('tar');

require('shelljs/global');

if (!which('cmake')) {
  console.error("[ERROR] CMake is required for installation.");
  exit(1);
}

function passthru() {
  var args = Array.prototype.slice.call(arguments);
  var cb = args.splice(-1)[0];
  var cmd = args.splice(0, 1)[0];
  var child = exec(cmd, { async: true });

  child.stdout.pipe(process.stdout);
  child.stderr.pipe(process.stderr);
  child.on("exit", cb);
}

var libgit2Dir = path.join(__dirname, "deps/libgit2/");

async.series([
  function(cb) {
    console.log("[gitteh] Downloading libgit2 dependency.");

    if (fs.existsSync(path.join(__dirname, '.git'))) {
      console.log("[gitteh] ...via git submodule");
      passthru("git submodule update --init", cb);
    } else {
      console.log("[gitteh] ...via tarball");
      var libgit2Version = "v0.19.0";
      var url = "https://github.com/libgit2/libgit2/tarball/" + libgit2Version;
      request.get(url).pipe(zlib.createUnzip()).pipe(tar.Extract({path: libgit2Dir, strip: true})).on('end', cb);
    }

  }
], function(err) {
	if(err) process.exit(err);
});
