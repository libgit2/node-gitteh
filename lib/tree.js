function Tree (utils) {

  return function (repository, obj) {
    var _this = this;
    this.repository = repository;

    obj._entries = obj.entries;
    obj.entries = [];

    _ref = obj._entries;
    for (_i = 0, _len = _ref.length; _i < _len; _i++) {
      origEntry = _ref[_i];
      obj.entries.push(entry = {});

      utils._immutable(entry, origEntry)
            .set('id')
            .set('name')
            .set('type')
            .set('attributes');
    }

    utils._immutable(_this, obj)
          .set('id')
          .set('entries');

  };
};

module.exports.all = function (Gitteh, utils) {
  Gitteh.Tree = Tree(utils);
};
