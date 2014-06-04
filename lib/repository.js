function Repository (Gitteh, utils, args) {

  return function (nativeRepo) {
    this.utils = utils;
    this.args = args;
    this.types = Gitteh.bindings.types;
    this.Commit = Gitteh.Commit;
    this.Blob = Gitteh.Blob;
    this.Tag = Gitteh.Tag;
    this.Tree = Gitteh.Tree;
    this.reference = Gitteh.Reference;

    if(!(nativeRepo instanceof Gitteh.bindings.NativeRepository)) {
      throw new Error('Don\'t construct me, see gitteh (open | init) Repository');
    }

    var _priv = this.utils._createPrivate(this);
    _priv.native = nativeRepo;

    this.utils._immutable(this, nativeRepo)
          .set('bare')
          .set('path')
          .set('workDir', 'workingDirectory')
          .set('remotes')
          .set('references')
          .set('submodules');

    var index = new Gitteh.Index(nativeRepo.Index);
    this.utils._immutable(this, {index: index})
          .set('index');
  };
};

Repository.prototype.exists = function (oid, cb) {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
          oid: {
            type: 'oid'
          },
          cb: {
            type: 'function'
          }
        })
      , oid = _ref[0]
      , cb = ref[1];

  return _priv.native.exists(oid, cb);
};

Repository.prototype.object = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        oid: {
          type: 'oid'
        },
        type: {
          type: 'objectType',
          default: 'any'
        },
        cb: {
          type: 'function'
        }
      })
    , oid = _ref[0]
    , type = _ref[1]
    , cb = _ref[2]
    , _this = this;

  _priv.native.object(oid, type, this.utils._wrapCallback(cb, function (object) {
    var clazz;
    switch (object._type) {
      case _this.types.commit:
        clazz = _this.Commit;
      case _this.types.tree:
        clazz = _this.Tree;
      case _this.types.blob:
        clazz = _this.Blob;
      case _this.types.tag:
        clazz = _this.Tag;
      default:
        break;
    };

    if (clazz === "undefined") {
      return cb(new TypeError("Unexpected Object Type"));
    }

    return cb(null, new clazz(_this, object));
  }));
};

Repository.prototype.blob = function (oid, cb) {
  return this.object(oid, 'blob', cb);
};

Repository.prototype.commit = function (oid, cb) {
  return this.object(oid, 'commit', cb);
};

Repository.prototype.tag = function (oid, cb) {
  return this.object(oid, 'tag', cb);
};

Repository.prototype.tree = function (oid, cb) {
  return this.object(oid, 'tree', cb);
};

Repository.prototype.reference = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        name: {
          type: 'string'
        },
        resolve: {
          type: 'bool',
          default: false
        },
        cb: {
          type: 'function'
        }
      })
    , name = _ref[0]
    , resolve = _ref[1]
    , cb = _ref[2]
    , _this = this;

  return _priv.native.reference(name, resolve, this.utils._wrapCallback(cb, function (ref) {
    return cb(null, new _this.reference(_this, ref));
  }));
};

Repository.prototype.createReference = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        name: {
          type: 'string'
        },
        target: {
          type: 'string'
        },
        force: {
          type: 'bool',
          default: false
        },
        cb: {
          type: 'function'
        }
      })
    , name = _ref[0]
    , target = _ref[1]
    , force = _ref[2]
    , cb = _ref[3]
    , _this = this;

  var fn = "createSymReference";
  if (target.length === 40 && this.args.oidRegex.test(target)) {
    fn = "createOidReference";
  }

  return _priv.native[fn](name, target, force, this.utils._wrapCallback(cb, function (ref) {
    return cb(null, new _this.reference(_this, ref));
  }));
};

Repository.prototype.remote = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        name: {
          type: 'string'
        },
        cb: {
          type: 'function'
        }
      })
    , name = _ref[0]
    , cb = _ref[1]
    , _this = this;

  return _priv.native.remote(name, this.utils._wrapCallback(cb, function (remote) {
    return cb(null, new Gitteh.Remote(_this, remote));
  }));
};

Repository.prototype.createRemote = function () {
  var _priv = this.utils._getPrivate(this)
      , _ref = this.args({
        name: {
          type: 'string'
        },
        url: {
          type: 'string'
        },
        cb: {
          type: 'function'
        }
      })
    , name = _ref[0]
    , url = _ref[1]
    , cb = _ref[2];

  return _priv.native.createRemote(name, url, this.utils._wrapCallback(cb, function (remote) {
    return cb(null, new Gitteh.Remote(_this, remote));
  }));
};

Repository.prototype.ref = Repository.prototype.reference;

module.exports.all = function (Gitteh, utils, args) {
  Gitteh.Repository = Repository(Gitteh, utils, args);
  Gitteh.Repository.prototype = Repository.prototype;
};
