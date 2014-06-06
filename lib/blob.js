function Blob (utils) {

  return function (repository, obj) {
    var _this = this;
    this.repository = repository;

    utils._immutable(_this, obj)
          .set('id')
          .set('data');
  };

};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Blob = Blob(utils);
}
