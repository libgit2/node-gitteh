function Refspec(utils) {

  return function (src, dst) {
    var _this = this
        , _priv = utils._createPrivate(_this);

    _priv.srcRoot = (src !== null) && src.slice(-1) === "*" ? src.slice(0, -1) : src;
    _priv.dstRoot = (dst !== null) && dst.slice(-1) === "*" ? dst.slice(0, -1) : dst;

    utils._immutable(_this, {src: src, dst: dst})
          .set('src')
          .set('dst');
  };
};

Refspec.prototype.matchesSrc = function (refName) {
  var _this = this
      , _priv = utils._getPrivate(_this);

  if (refName.length <= _priv.srcRoot.length)
    return false;

  return refName.indexOf(_priv.srcRoot) === 0;
};

Refspec.prototype.matchesDst = function (refName) {
  var _this = this
      , _priv = utils._getPrivate(_this);

  if (refName.length <= _priv.dstRoot.length)
    return false;

  return refName.indexOf(_priv.dstRoot) === 0;
};

Refspec.prototype.transformTo = function (refName) {
  if (!this.matchesSrc(refName))
    throw new Error('Ref doesn\'t match with src.');

  return "" + this.dst.slice(0, -2) + refName.slice(this.src.length - 2);
};

Refspec.prototype.transformFrom = function (refName) {
  if(!this.matchesDst(refName))
    throw new Error('Ref doesn\'t match with dst.');

  return "" + this.src.slice(0, -2) + refName.slice(this.dst.length - 2);
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Refspec = Refspec(utils);
};
