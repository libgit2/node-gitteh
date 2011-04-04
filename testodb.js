var gitteh = require("gitteh");
var path = require("path");
var fs = require("fs");

var odbDir = path.join(__dirname, "testpacked", ".git", "objects");

fs.renameSync(
		path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.pack"),
		path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.old")
);

var s = fs.createWriteStream(path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.pack"));
s.end("");

try {
var odb = gitteh.openODB(odbDir);
}
catch(e) {
	console.log(e.gitErrorStr);
	throw e;
}

console.log(odb.exists("69906ba14f17bb026db62043e2359c97778ab5e7"));

fs.unlinkSync(path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.pack"));

fs.renameSync(
		path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.old"),
		path.join(odbDir, "pack", "pack-1d97ccb826e2beb153608f3ad746bce68ea542e0.pack")
);

console.log(odb.exists("69906ba14f17bb026db62043e2359c97778ab5e7"));

//69906ba14f17bb026db62043e2359c97778ab5e7