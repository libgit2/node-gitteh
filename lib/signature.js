function Signature (utils) {

  return function (obj) {
    var _this = this;
    
    utils._immutable(_this, obj)
          .set('name')
          .set('email')
          .set('time')
          .set('offset');
  };

};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Signature = Signature(utils);
};
