function getPrivate (obj) {
  getPrivate.lock++;
  return obj._private;
};

getPrivate.lock = 0;

function defineObject (obj, prop, opts) {
  Object.defineProperty(obj, prop,
    {
      enumerable: opts.enumerable,
      configurable: opts.configurable,
      get: opts.get
  });
};

function createPrivate (obj) {
  var opts = {}
      ,_priv = {};

  opts.get = function () {
    if (!getPrivate.lock--) {
      throw new Error("Bad Request.");
    }
    return _priv;
  };

  defineObject(obj, '_private', opts);
  return _priv;
};

function wrapCallback (original, callback) {
  return function (err) {
    if (err !== null) return original(err);

    return callback.apply(null, Array.prototype.slice.call(arguments, 1));
  };
};

function immutable (obj, src) {
  var opts = {}, result;

   return result = {
      set: function(name, target) {
         if (target === undefined) target = name;
         if (Array.isArray(src[name])) {
           Object.defineProperty(obj, target, {
             get: function() {
             return src[name].slice(0);
            },
             configurable: false,
             enumerable: true
           });
          return result;
        }

       Object.defineProperty(obj, target, {
         value: src[name],
         writable: false,
           configurable: false,
           enumerable: true
         });

      return result;
    }
  };

};

module.exports = {
  _getPrivate: getPrivate,
  _createPrivate: createPrivate,
  _wrapCallback: wrapCallback,
  _immutable: immutable,
  _defineObject: defineObject
};
