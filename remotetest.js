var gitteh = require("./lib/gitteh");

var path = require("path").join(__dirname, "remotetest");
try {
	require("wrench").rmdirSyncRecursive(path);
}catch(e) {console.log(e); };

/*
//gitteh.Remote.clone("git://github.com/libgit2/node-gitteh.git", "/blah");

gitteh.Remote.clone("https://github.com/libgit2/node-gitteh.git", path, function(err, result) {
	console.log(arguments);
});

/*gitteh.Remote.clone("https://github.com/samcday/asyncevents.git", path, function(err, result) {
	console.log(arguments);
});*/


gitteh.Remote.clone("git://github.com/libgit2/node-gitteh.git", path, function(err, repo) {
	console.log("Clone result", arguments);
});


/*gitteh.Remote.clone("git://github.com/git/git.git", path, function(err, repo) {
console.log("Clone result", arguments);
});*/
