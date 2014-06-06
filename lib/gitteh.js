var bindings = require("bindings")("gitteh.node")
    , utils        = require("./utils")
    , args       = require("./args");

var helpers = ['signature', 'refspec', 'commit', 'tree', 'blob', 'tag', 'remote', 'index', 'reference', 'repository'];

var Gitteh = module.exports = {};

Gitteh.bindings = bindings
  , Gitteh.args = args
  , Gitteh.utils = utils
  , args.minOidLength = bindings.minOidLength;

Object.keys(helpers).forEach(function (fileName) {
  var helper = helpers[fileName];
  require("./" + helper).all(Gitteh, utils, args);
});

Gitteh.openRepository = function () {
  var _ref = args({
    path: {
      type: 'string'
    },
    cb: {
      type: 'function'
    }
  })
    , path = _ref[0]
    , cb = _ref[1];

  return bindings.openRepository(path, utils._wrapCallback(cb, function (repo) {
    return cb(null, new Gitteh.Repository(repo));
  }));
};

Gitteh.openIndex = function () {
  var _ref = args({
    repo: {
      type: "repository"
    },
    cb: {
      type: "function"
    }
  })
    , repo = _ref[0]
    , cb = _ref[1];

  return bindings.openIndex(repo, utils._wrapCallback(cb, function (repo) {
    return cb(null, new Gitteh.Index(repo));
  }));
}

Gitteh.initRepository = function () {
  var _ref = args({
    path: {
      type: 'string'
    },
    bare: {
      type: 'bool',
      "default": false
    },
    cb: {
      type: 'function'
    }
  })
  , path = _ref[0]
  , bare = _ref[1]
  , cb = _ref[2];

  return bindings.initRepository(path, bare, utils._wrapCallback(cb, function (repo) {
    return cb(null, new Gitteh.Repository(repo));
  }));
};

Gitteh.clone = function () {

};
