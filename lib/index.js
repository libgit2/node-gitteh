function Index (utils, args) {

  return function (nativeIndex) {
    this.utils = utils;
    this.args = args;

    var _priv = utils._createPrivate(this);

    _priv.native = nativeIndex;
  };

};

Index.prototype.readTree = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
          id: {
            type: "oid"
          },
          cb: {
            type: "function"
          }
        })
      , id = _ref[0]
      , cb = _ref[1];

  return _priv.native.readTree(id, cb);
};

Index.prototype.write = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        cb: {
          type: "function"
        }
      })
      , cb = _ref[1];

  return _priv.native.write(cb);      
};

module.exports.all = function (Gitteh, utils, args) {
  Gitteh.Index = Index(utils, args);
}
