function Reference (utils) {

  return function (repo, nativeRef) {
    var _this = this
        , _priv = utils._createPrivate(_this)

    _priv.native = nativeRef;
    utils._immutable(_this, nativeRef)
          .set('name')
          .set('direct')
          .set('packed')
          .set('target');

    utils._immutable(_this, {repo: repo})
            .set('repo', 'repository');

  };
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Reference = Reference(utils);
};
