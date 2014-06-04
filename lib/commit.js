function Commit (Gitteh, utils) {

  return function (repository, obj) {
      this.repository = repository;

      obj.author = new Gitteh.Signature(obj.author);
      obj.committer = new Gitteh.Signature(obj.committer);

      utils._immutable(this, obj)
            .set('id')
            .set('tree', 'treeId')
            .set('parents')
            .set('message')
            .set('messageEncoding')
            .set('author')
            .set('committer');
  };

};

Commit.prototype.tree = function (cb) {
  return this.repository.tree(this.treeId, cb);
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Commit = Commit(Gitteh, utils);
};
