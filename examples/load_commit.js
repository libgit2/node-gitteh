var gitteh = require("../lib/gitteh");
var path = require("path");

var meh = null;
setTimeout(function() {
	gc();
	console.log(meh.id);
	delete meh;
	meh = null;
}, 500);
setTimeout(function() {
	gc();
	gc();
}, 1000);

setTimeout(function() {
}, 1500);

gitteh.openRepository(path.join(__dirname, ".."), function(err, repo) {
	repo.commit("1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", function (err, commit) {
		meh = commit;
		console.log(commit);
	});
});
