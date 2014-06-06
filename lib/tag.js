function Tag (Gitteh, utils) {

  return function (repository, obj) {
    var _this = this;
    this.repository = repository;

    obj.tagger = new Gitteh.Signature(obj.tagger);
    utils._immutable(_this, obj)
          .set('id')
          .set('name')
          .set('message')
          .set('tagger')
          .set('target', 'targetId')
          .set('type');
  };

};

Tag.prototype.target = function () {
  return this.repository.object(this.targetId, this.type, cb);
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Tag = Tag(Gitteh, utils);
};
