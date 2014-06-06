var git, types;

function Repository (Gitteh, utils, args) {
  git = Gitteh;
  types = Gitteh.bindings.types;

  return function (nativeRepo) {
    var _this = this;

    if(!(nativeRepo instanceof Gitteh.bindings.NativeRepository)) {
      throw new Error('Don\'t construct me, see gitteh (open | init) Repository');
    }

    var _priv = git.utils._createPrivate(_this);
    _priv.native = nativeRepo;

    git.utils._immutable(_this, nativeRepo)
          .set('bare')
          .set('path')
          .set('workDir', 'workingDirectory')
          .set('remotes')
          .set('references')
          .set('submodules');

    var index = new Gitteh.Index(nativeRepo.Index);
    git.utils._immutable(_this, {index: index})
          .set('index');
  };
};

Repository.prototype.exists = function (oid, cb) {
  var _this = this
      , _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
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
  var _this = this
      , _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
        oid: {
          type: 'oid'
        },
        type: {
          type: 'objectType',
          'default': 'any'
        },
        cb: {
          type: 'function'
        }
      })
    , oid = _ref[0]
    , type = _ref[1]
    , cb = _ref[2];

  return _priv.native.object(oid, type, git.utils._wrapCallback(cb, function (object) {
        var  clazz = (function() {
           switch (object._type) {
             case types.commit:
               return git.Commit;
             case types.tree:
               return git.Tree;
             case types.blob:
               return git.Blob;
             case types.tag:
               return git.Tag;
             default:
               return;
           }
         })();

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
  var _this = this
      ,  _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
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
    , cb = _ref[2];

  return _priv.native.reference(name, resolve, git.utils._wrapCallback(cb, function (ref) {
    return cb(null, new git.Reference(_this, ref));
  }));
};

Repository.prototype.createReference = function () {
  var _this = this
      , _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
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
    , cb = _ref[3];

  var fn = "createSymReference";
  if (target.length === 40 && git.args.oidRegex.test(target)) {
    fn = "createOidReference";
  }

  return _priv.native[fn](name, target, force, git.utils._wrapCallback(cb, function (ref) {
    return cb(null, new git.Reference(_this, ref));
  }));
};

Repository.prototype.remote = function () {
  var _this = this
      , _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
        name: {
          type: 'string'
        },
        cb: {
          type: 'function'
        }
      })
    , name = _ref[0]
    , cb = _ref[1];

  return _priv.native.remote(name, git.utils._wrapCallback(cb, function (remote) {
    return cb(null, new git.Remote(_this, remote));
  }));
};

Repository.prototype.createRemote = function () {
  var _this = this
      , _priv = git.utils._getPrivate(_this)
      , _ref = git.args({
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

  return _priv.native.createRemote(name, url, git.utils._wrapCallback(cb, function (remote) {
    return cb(null, new git.Remote(_this, remote));
  }));
};

Repository.prototype.ref = Repository.prototype.reference;

module.exports.all = function (Gitteh, utils, args) {
  Gitteh.Repository = Repository(Gitteh, utils, args);
  Gitteh.Repository.prototype = Repository.prototype;
};
