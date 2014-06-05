var objectTypes = ["any", "blob", "commit", "tag", "tree"]
    , remoteDirs  = ["push", "fetch"]
    , __hasProp = {}.hasOwnProperty
    , numRequired, count;

function Args (params) {

  var paramList = [], param, retlLeft, retRight
      , retParam = [];

  for (name in params) {
    if (!params.hasOwnProperty(name)) continue;

    param = params[name];
    param.name = name;

    paramList.push(param);
  }

  numRequired = 0;
  count = 0;

  leftRequired = (function() {
    var res = [];

    for (var i = 0, len = paramList.length; i < len; i++) {

      if (paramList[i].hasOwnProperty("default")) {
        break;
      }

      numRequired++;
      res.push(paramList[i]);
    }

    return res;
  })();

  paramList.splice(0, leftRequired.length);

  rightRequired = (function() {
    var _ref = paramList.slice(0).reverse()
        , res = [];

    for (var i = 0, len = _ref.length; i < len; i++) {

      if (_ref[i].hasOwnProperty("default")) {
        break;
      }

      numRequired++;
      res.push(_ref[i]);
    }

    return res;
  })();

  rightRequired = rightRequired.reverse();
  paramList.splice(-rightRequired.length);
  argList = Array.prototype.slice.call(Args.caller["arguments"]);

  if (argList.length < numRequired) {
    throw new Error("Not enough arguments.");
  }

  left = argList.splice(0, leftRequired.length);
  right = argList.splice(-rightRequired.length);

  for (var i = 0, len = left.length; i < len; i++) {
    count++;

    if (!Args.validators[leftRequired[i].type](left[i])) {
      throw new TypeError("" + leftRequired[i].name + " (" + count + ") is not a valid " + leftRequired[i].type);
    }

    retParam.push(left[i]);
  };

  for (var i = 0, len = paramList.length; i < len; i++) {
    count++;

    if (argList.length > i) {

      if (!Args.validators[paramList[i].type](argList[i])) {
        throw new TypeError("" + paramList[i].name + " (" + count + ") is not a valid " + paramList[i].type);
      }
      retParam.push(argList[i]);

    } else {
      retParam.push(paramList["default"]);
    }

  };

  for (var i = 0, len = right.length; i < len; i++) {
    count++;

    if (!Args.validators[rightRequired[i].type](right[i])) {
      throw new TypeError("" + rightRequired[i].name + " (" + count + ") is not a valid " + rightRequired[i].type);
    }

    retParam.push(right[i]);
  };

  return retParam;
};

Args.oidRegex = oidRegex = /^[a-zA-Z0-9]{0,40}$/;

Args.validators = {
  string: function (val) {
    return typeof val === "string";
  },
  function: function (val) {
    return typeof val === "function";
  },
  bool: function (val) {
    return typeof val === "boolean";
  },
  oid: function (val) {
    if (typeof val !== "string" && !oidRegex.test(val) && val.length < Args.minOidLength) {
      return false;
    }
    return true;
  },
  objectType: function (val) {
    return objectTypes.indexOf(val) > -1;
  },
  remoteDir: function (val) {
    return remoteDirs.indexOf(val) > -1;
  }
};

module.exports = Args;
