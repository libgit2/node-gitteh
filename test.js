var blah = require("./build/default/hello");

var myRepo = new blah.Repository();

console.log(myRepo);

console.log(myRepo.setSomething("Weee"));

console.log(myRepo);