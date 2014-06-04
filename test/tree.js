module.exports.all = function (gitteh, test) {

  test('can find tree of last commit on master...', function (t) {

    gitteh.openRepository('test/repo/workdir/.git', function (err, repo) {
      repo.commit("60e0dbe58458ed42d0191a1780d91e14b8b7e0be", function (err, tree) {

        repo.tree("40040fbc2c1bcfaa96c63e2a11aed480fccb6db8", function (err, tree) {
          //t.ok(tree instanceof gitteh.Tree, 'is an instance of #gitteh.Tree');
          t.equal(tree.id, '40040fbc2c1bcfaa96c63e2a11aed480fccb6db8', 'returns tree id');

          t.end();
        });

      });
    });

  });

};
