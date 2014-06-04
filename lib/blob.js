function Blob (utils) {

  return function (repository, obj) {
    this.repository = repository;

    utils._immutable(this, obj)
          .set('id')
          .set('data');
  };

};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Blob = Blob(utils);
}
