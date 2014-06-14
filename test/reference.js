var path = require('path')
    , fs     = require('fs');

module.exports.all = function (gitteh, test) {

  test('can find the HEAD sym reference', function (t) {
    t.plan(6);

    gitteh.openRepository('test/repo/workdir/.git', function(err, repo) {
      var headPath = path.join(repo.path, "HEAD")
          , headFile   = fs.readFileSync(headPath, 'utf-8').slice(5, -1);

      repo.ref("HEAD", false, function (err, ref) {
        t.ok(ref instanceof gitteh.Reference, 'returns an instance of #gitteh.Reference');
        t.equal(ref.name, 'HEAD', 'has valid property #name');
        t.equal(ref.target, headFile, 'points to .git/HEAD');
        t.notOk(ref.direct, 'direct should be false');
      });

      t.test('gives resolved direct ref', function (t) {
        repo.ref("HEAD", true, function (err, ref) {
          t.error(err, 'shouldnt throw an exception')
          t.equal(ref.name, headFile);
          t.end();
        });
      });

      t.test('create direct ref', function (t) {
        fs.unlink(path.join(repo.path, "refs", "heads", "testref"));

        repo.createReference('refs/heads/testref', '60e0dbe58458ed42d0191a1780d91e14b8b7e0be', function (err, newref) {
          t.error(err, 'shouldnt throw an exception');
          t.ok(newref instanceof gitteh.Reference, 'returns an instance of #gitteh.Reference');
          t.equal(newref.name, 'refs/heads/testref', 'contains valid target name');
          t.equal(newref.target, '60e0dbe58458ed42d0191a1780d91e14b8b7e0be', 'points to commit id');
          t.end();
        });
      });

    });
  });
};
