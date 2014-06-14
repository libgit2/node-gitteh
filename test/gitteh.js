var temp    = require('temp')
    , path     = require('path')
    , wrench = require('wrench');

module.exports.all = function (gitteh, test) {
  test('opening a valid repo...', function (t) {
    gitteh.openRepository("test/repo/workdir/.git", function (err, repo) {
      t.ok(repo instanceof gitteh.Repository, 'returns an instance of #gitteh.Repository');
      t.end();
    });
  });
/*
  test('opening an invalid repo...', function (t) {
    gitteh.openRepository("/i/shouldnt/exist", function (err) {
      t.throws(err, 'throws an error');
      t.end();
    });
  });

  test('initialising on an invalid path...', function (t) {
    gitteh.initRepository("/i/shouldnt/exist", function (err) {
      t.throws(err, 'throws an exception');
      t.end();
    });
  });
*/
  test('initialising a bare repo...', function (t) {
    var tempPath = temp.path();

    gitteh.initRepository(tempPath, true, function (err, repo) {
      t.ok(repo instanceof gitteh.Repository, 'returns an instance of #gitteh.Repository');
      t.ok(repo.bare, 'works');
      t.equal(path.relative(repo.path, tempPath), '', 'initialises a bare repo in the specified path');

      wrench.rmdirSyncRecursive(tempPath, true);

      t.end();
    });
  });
};
