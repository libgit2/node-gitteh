function Signature (utils) {

  return function (obj) {
    utils._immutable(this, obj)
          .set('name')
          .set('email')
          .set('time')
          .set('offset');
  };
  
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Signature = Signature(utils);
};
