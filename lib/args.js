(function() {
  var fn, objectTypes, oidRegex, remoteDirs,
    __hasProp = Object.prototype.hasOwnProperty;

  module.exports = fn = function(params) {
    var arg, argn, args, i, left, leftRequired, name, numRequired, param, paramList, ret, right, rightRequired, _len, _len2, _len3;
    paramList = (function() {
      var _results;
      _results = [];
      for (name in params) {
        if (!__hasProp.call(params, name)) continue;
        param = params[name];
        param.name = name;
        _results.push(param);
      }
      return _results;
    })();
    ret = [];
    numRequired = 0;
    leftRequired = (function() {
      var _i, _len, _results;
      _results = [];
      for (_i = 0, _len = paramList.length; _i < _len; _i++) {
        param = paramList[_i];
        if (param.hasOwnProperty("default")) break;
        numRequired++;
        _results.push(param);
      }
      return _results;
    })();
    paramList.splice(0, leftRequired.length);
    rightRequired = (function() {
      var _i, _len, _ref, _results;
      _ref = paramList.slice(0).reverse();
      _results = [];
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        param = _ref[_i];
        if (param.hasOwnProperty("default")) break;
        numRequired++;
        _results.push(param);
      }
      return _results;
    })();
    rightRequired = rightRequired.reverse();
    paramList.splice(-rightRequired.length);
    args = Array.prototype.slice.call(fn.caller.arguments);
    if (args.length < numRequired) throw new Error("Not enough arguments.");
    left = args.splice(0, leftRequired.length);
    right = args.splice(-rightRequired.length);
    argn = 0;
    for (i = 0, _len = left.length; i < _len; i++) {
      arg = left[i];
      argn++;
      param = leftRequired[i];
      if (!fn.validators[param.type](arg)) {
        throw new TypeError("" + param.name + " (" + argn + ") is not a valid " + param.type);
      }
      ret.push(arg);
    }
    for (i = 0, _len2 = paramList.length; i < _len2; i++) {
      param = paramList[i];
      argn++;
      if (args.length > i) {
        arg = args[i];
        if (!fn.validators[param.type](arg)) {
          throw new TypeError("" + param.name + " (" + argn + ") is not a valid " + param.type);
        }
        ret.push(arg);
      } else {
        ret.push(param["default"]);
      }
    }
    for (i = 0, _len3 = right.length; i < _len3; i++) {
      arg = right[i];
      argn++;
      param = rightRequired[i];
      if (!fn.validators[param.type](arg)) {
        throw new TypeError("" + param.name + " (" + argn + ") is not a valid " + param.type);
      }
      ret.push(arg);
    }
    return ret;
  };

  fn.oidRegex = oidRegex = /^[a-zA-Z0-9]{0,40}$/;

  objectTypes = ["any", "blob", "commit", "tag", "tree"];

  remoteDirs = ["push", "fetch"];

  fn.validators = {
    string: function(val) {
      return typeof val === "string";
    },
    "function": function(val) {
      return typeof val === "function";
    },
    bool: function(val) {
      return typeof val === "boolean";
    },
    oid: function(val) {
      if (typeof val !== "string") return false;
      if (!oidRegex.test(val)) return false;
      if (val.length < fn.minOidLength) return false;
      return true;
    },
    objectType: function(val) {
      return objectTypes.indexOf(val > -1);
    },
    remoteDir: function(val) {
      return remoteDirs.indexOf(val > -1);
    }
  };

}).call(this);
