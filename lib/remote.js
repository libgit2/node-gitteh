function Remote (utils, args) {

  return function (repository, nativeRemote) {
    this.repository = repository;
    this.utils = utils;
    this.args = args;

    var _priv = utils._createPrivate(this)
        , opts = {};

    _priv.native = nativeRemote;
    _priv.connected = false;

    if (!(nativeRemote instanceof Gitteh.bindings.NativeRemote)) {
      throw new Error('Don\'t construct me, see Repository.remote()');
    }

    opts.connected = _priv.connected;
    opts.enumerable = true;
    opts.get = function () {
      return opts.connected;
    };

    utils._defineObject(this, 'connected', opts);

    utils._immutable(this, nativeRemote)
          .set('name')
          .set('url');

    var fetchSpec = new RefSpec(nativeRemote.fetchSpec.src, nativeRemote.fetchSpec.dst)
        , pushSpec = new RefSpec(nativeRemote.pushSpec.src, nativeRemote.pushSpec.dst);

    utils._immutable(this, {fetchSpec: fetchSpec, pushSpec: pushSpec})
          .set('fetchSpec')
          .set('pushSpec');
  };
};

Remote.prototype.connect = function () {
  var _priv = this.utils._getPrivate(this)
      ,  _ref = this.args({
        dir: {
          type: "remoteDir"
        },
        cb: {
          type: "function"
        }
      })
    , dir = _ref[0]
    , cb = _ref[1]
    , _this = this;

    dir = dir === "push" ? Gitteh.bindings.GIT_DIRECTION_PUSH : Gitteh.bindings.GIT_DIRECTION_FETCH;

    return _priv.native.connect(dir, this.utils._wrapCallback(cb, function (refs) {
      var refNames = Object.keys(refs)
          , headOid   = refs["HEAD"];

      for (ref in refs) {
        var oid = refs[ref];
        if (ref === "HEAD") continue;

        if (oid === headOid) {
          var headRef = this.fetchSpec.transformTo(ref);
          _this.utils._immutable(this, {headRef: headRef})
                        .set("headRef", "HEAD");
          break;
        }
      };

      _this.utils._immutable(this, {refNames: refNames})
                    .set("refNames", "refs");
      _priv.connected = true;
      cb();
    }));

};

Remote.prototype.fetch = function () {
  var _priv = this.utils._getPrivate(this)
      , updateTimer = null;

  if(!this.connected) {
    throw new Error('Remote isn\'t connected.');
  }

  var _ref = this.args({
    progressCb: {
      type: "function"
    },
    cb: {
      type: "function"
    }
  })
    , progressCb = _ref[0]
    , cb = _ref[1]

    var update = function () {
      var progress = _priv.native.stats;
      progressCb(progress.bytes, progress.total, progress.done);
      updateTimer = setTimeout(update, 500);
    };

    setTimeout(update, 500);

    _priv.native.download = function () {
      clearTimeout(updateTimer);
      if (err) {
        return cb(err);
      }

      _priv.native.updateTips(this.utils._wrapCallback(cb, function () {
        cb();
      }));
    };

    return _priv.native.download;
};

module.exports.all = function (Gitteh, utils, args) {
  Gitteh.Remote = Remote(utils, args);
  Gitteh.Remote.prototype = Remote.prototype;
};
