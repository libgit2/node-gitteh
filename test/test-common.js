var path   = require('path')
  ,   fs       = require('fs')
  ,   exec   = require('child_process').exec;

var setupRepo = function (callback) {
  fs.exists(path.join(__dirname, 'repo'), function (exists) {

    if (exists) return callback();

    fs.mkdir(path.join(__dirname, 'repo'), function () {
      exec('git clone https://github.com/libgit2/node-gitteh.git ./test/repo/workdir', function (err) {
        return callback(err);
      });
    });

  });
}

  , setup = function (t) {
      setupRepo(function (err) {
        if (err) t.notOk(err, 'setup returned an error');
        t.end();
      });
    };

module.exports = {
  setUp : setup
};
