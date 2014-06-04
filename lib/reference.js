function Reference (utils) {

  return function (repo, nativeRef) {
    var _priv = utils._createPrivate(this);

    _priv.native = nativeRef;
    utils._immutable(this, nativeRef)
          .set('name')
          .set('direct')
          .set('packed')
          .set('target');
    utils._immutable(this, {repo: repo})
            .set('repo', 'repository');
            
  };
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Reference = Reference(utils);
};
