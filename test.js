var gitteh = require("gitteh"),
	path = require("path");
	
var repo = gitteh.openRepository(path.join(__dirname, ".git"));
var i = 0;

function gogc() {
	//gc();
}

function go() {
	if(i++ > 1000) return;

try {
	repo.getCommit("b9561bc355a20f68035eb5c2d97b1d6e7a715b96");	
}
catch(e) {
	console.log(e);
}

try {
	repo.getCommit("b9561bc355a20f68035eb5c2d97b1d6e7a715b96");	
}
catch(e) {
	console.log(e);
}


gogc();
gogc();
repo.getCommit("b9561bc355a20f68035eb5c2d97b1d6e7a715b96", function() {
	gogc();
});

gogc();
repo.getCommit("b9561bc355a20f68035eb5c2d97b1d6e7a715b96");	
gogc();

repo.getCommit("b9561bc355a20f68035eb5c2d97b1d6e7a715b96", function() {

	gogc();
});
gogc();
gogc();
gogc();


process.nextTick(go);
}

go();