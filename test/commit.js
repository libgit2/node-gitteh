module.exports.all = function (gitteh, test) {

  test('can find the commit on master...', function (t) {
    t.plan(2);

    gitteh.openRepository('test/repo/workdir/.git', function (err, repo) {
      repo.commit('60e0dbe58458ed42d0191a1780d91e14b8b7e0be', function (err, commit) {
        t.error(err, 'shoudnt throw an exception');
        //t.ok(commit instanceof gitteh.Commit, 'returns an instance of #gitteh.Commit');
        t.equal(commit.id, '60e0dbe58458ed42d0191a1780d91e14b8b7e0be');
        //t.equal(commit.parents.length, 1, 'contains a array with one parent');

        /*
        t.test('validity of author...', function (t) {
            var author = commit.author;

            t.equal(author.name, 'Sam', 'name');
            t.equal(author.email, 'me@samcday.com.au', 'email');
            t.ok(author.offset, 'contains offset property');
            t.ok(author.time, 'contains time of commit');
            t.end();
        });

        t.test('validity of committer...', function (t) {
            var committer = commit.committer;

            //t.equal(committer.name, 'Sam', 'name');
            t.equal(committer.email, 'me@samcday.com.au', 'email');
            t.ok(committer.offset, 'contains offset property');
            t.ok(committer.time, 'contains time of commit');
            t.end();
        });
        */

      });
    });

  });
};
