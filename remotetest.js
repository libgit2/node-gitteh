var gitteh = require("./lib/gitteh");

try {
	require("wrench").rmdirSyncRecursive("/blah");
}catch(e) {console.log(e); };
//gitteh.Remote.clone("git://github.com/libgit2/node-gitteh.git", "/blah");
gitteh.Remote.clone("https://github.com/libgit2/node-gitteh.git", "/blah");