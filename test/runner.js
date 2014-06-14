const test               = require('tape')
        , testCommon = require('./test-common')
        , gitteh            = require('../lib/gitteh');

var testFiles = ['gitteh', 'tree', 'reference', 'commit'];

test('setup common', testCommon.setUp);

Object.keys(testFiles).forEach(function (fileName) {
  var testFile = testFiles[fileName];
  require("./" + testFile).all(gitteh, test);
});
