/*
	This file provides wrappers for the actual native bindings and performs additional validation of arguments 
	and other types of error and exception handling that are more cumbersome to do in C++. You should require
	this file rather than requiring the binary of the binding directly. 
*/

(function() {

  EventEmitter = require("events").EventEmitter;
  async = require("async");
  fs = require("fs");
  _path = require("path");
  args = require("./args");
  env = "Release";
  bindings = require("../build/" + env + "/gitteh");

  var Blob, Commit, EventEmitter, Gitteh, Index, NativeRemote, NativeRepository, Reference, Refspec, Remote, Repository, Signature, Tag, Tree, args, async, bindings, env, fs, minOidLength, types, _createPrivate, _getPrivate, _immutable, _path, _wrapCallback,

   _this = this;

  if (env === "Debug") (require("segfault-handler")).registerHandler();

  minOidLength = bindings.minOidLength, types = bindings.types, NativeRepository = bindings.NativeRepository, NativeRemote = bindings.NativeRemote;

  args.minOidLength = minOidLength;

  Gitteh = module.exports = {};

  _getPrivate = function(obj) {
    _getPrivate.lock++;
    return obj._private;
  };

  _getPrivate.lock = 0;

  _createPrivate = function(obj) {
    var _priv;
    _priv = {};
    Object.defineProperty(obj, "_private", {
      enumerable: false,
      configurable: false,
      get: function() {
        if (!_getPrivate.lock--) throw new Error("Bad request");
        return _priv;
      }
    });
    return _priv;
  };

  _wrapCallback = function(orig, cb) {
    return function(err) {
      if (err != null) return orig(err);
      return cb.apply(null, Array.prototype.slice.call(arguments, 1));
    };
  };

  _immutable = function(obj, src) {
    var o;
    return o = {
      set: function(name, target) {
        if (target == null) target = name;
        if (Array.isArray(src[name])) {
          Object.defineProperty(obj, target, {
            get: function() {
              return src[name].slice(0);
            },
            configurable: false,
            enumerable: true
          });
          return o;
        }
        Object.defineProperty(obj, target, {
          value: src[name],
          writable: false,
          configurable: false,
          enumerable: true
        });
        return o;
      }
    };
  };

  Gitteh.Signature = Signature = (function() {
    /*
    	Contains the name/email/time for a :class:`gitteh::Commit` author/committer
    	or :class:`gitteh::Tag` tagger.
    
    	Signatures contain the following *immutable* properties:
    
    	- **name**: *(String)*
    	- **email**: *(String)*
    	- **time**: *(Date)*
    	- **offset**: *(Number)* timezone offset in seconds from GMT.
    */
    function Signature(obj) {
      _immutable(this, obj).set("name").set("email").set("time").set("offset");
    }

    return Signature;

  })();

  Gitteh.Refspec = Refspec = (function() {
    /*
    	Describes the way remote repository references will be mapped to the local
    	repository. 
    
    	For more information refer to http://git-scm.com/book/ch9-5.html
    */
    function Refspec(src, dst) {
      var _priv;
      _priv = _createPrivate(this);
      _priv.srcRoot = (src != null) && src.slice(-1) === "*" ? src.slice(0, -1) : src;
      _priv.dstRoot = (dst != null) && dst.slice(-1) === "*" ? dst.slice(0, -1) : dst;
      _immutable(this, {
        src: src,
        dst: dst
      }).set("src").set("dst");
    }

    Refspec.prototype.matchesSrc = function(refName) {
      /*
      		Returns true/false if given `refName` matches source of this Refspec.
      */
      var _priv;
      _priv = _getPrivate(this);
      if (refName.length <= _priv.srcRoot.length) return false;
      return refName.indexOf(_priv.srcRoot) === 0;
    };

    Refspec.prototype.matchesDst = function(refName) {
      /*
      		Returns true/false if given `refName` matches destination of this
      		Refspec.
      */
      var _priv;
      _priv = _getPrivate(this);
      if (refName.length <= _priv.dstRoot.length) return false;
      return refName.indexOf(_priv.dstRoot) === 0;
    };

    Refspec.prototype.transformTo = function(refName) {
      /*
      		Transforms given `refName` to destination, provided it matches src
      		pattern, and throws an error if it doesn't.
      */      if (!this.matchesSrc(refName)) throw new Error("Ref doesn't match src.");
      return "" + this.dst.slice(0, -2) + refName.slice(this.src.length - 2);
    };

    Refspec.prototype.transformFrom = function(refName) {
      /*
      		Transforms provided refName from destination back to source, provided it
      		matches dst pattern, and throws an Error if it doesn't. This is the
      		reverse of :func:`gitteh::Refspec.transformTo`
      */      if (!this.matchesDst(refName)) throw new Error("Ref doesn't match dst.");
      return "" + this.src.slice(0, -2) + refName.slice(this.dst.length - 2);
    };

    return Refspec;

  })();

  Gitteh.Commit = Commit = (function() {
    /*
    	Commits, made by an author, and an optional different committer, contain a
    	message, an associated :class:`gitteh::Tree`, and zero or more parent
    	:class:`gitteh::Commit` objects. Zero parents generally indicate the initial 
    	commit for the repository. More than one parent commits indicate a merge
    	commit.
    
    	Properties:
    
    	* **id**: *(String)* OID of this commit (SHA1 hash)
    	* **treeId**: *(String)* OID of associated :class:`gitteh::Tree`
    	* **parents**: *(String[]) list of parent commit OIDs
    	* **message**: *(String)*
    	* **messageEncoding**: *(???)* ??? TODO:
    	* **author**: (:class:`gitteh::Signature`)
    	* **committer**: (:class:`gitteh::Signature`)
    */
    function Commit(repository, obj) {
      this.repository = repository;
      obj.author = new Signature(obj.author);
      obj.committer = new Signature(obj.committer);
      _immutable(this, obj).set("id").set("tree", "treeId").set("parents").set("message").set("messageEncoding").set("author").set("committer");
    }

    Commit.prototype.tree = function(cb) {
      /*
      		Fetches the :class:`gitteh::Tree` for this Commit. Shortcut for calling
      		:func:`gitteh::Repository.tree` with this commits `treeId`.
      */      return this.repository.tree(this.treeId, cb);
    };

    return Commit;

  })();

  Gitteh.Tree = Tree = (function() {
    /*
    	A Tree contains a list of named entries, which can either be
    	:class:`gitteh::Blob` objects or nested :class:`gitteh::Tree` objects. Each
    	entry is referenced by its OID.
    
    	Properties:
    
    	* **id**: *(String)* OID of this Tree.
    	* **entries**: *(TreeEntry[])* 
    
    	## Tree Entries
    	
    	Each element of a Tree contains the following properties:
    
    	* **id**: *(String)* OID this entry points to.
    	* **name**: *(String)* file name of this entry.
    	* **type**: *(String)* kind of object pointed to by this entry
    	* **attributes**: *(Number)* UNIX file attributes for this entry.
    */
    function Tree(repository, obj) {
      var entry, origEntry, _i, _len, _ref;
      this.repository = repository;
      obj._entries = obj.entries;
      obj.entries = [];
      _ref = obj._entries;
      for (_i = 0, _len = _ref.length; _i < _len; _i++) {
        origEntry = _ref[_i];
        obj.entries.push(entry = {});
        _immutable(entry, origEntry).set("id").set("name").set("type").set("attributes");
      }
      _immutable(this, obj).set("id").set("entries");
    }

    return Tree;

  })();

  Gitteh.Blob = Blob = (function() {
    /*
    	Contains raw data for a file stored in Git.
    
    	Properties:
    
    	* **id**: *(String)* OID of this Blob.
    	* **data**: *(Buffer)* a standard Node buffer containing binary data.
    */
    function Blob(repository, obj) {
      this.repository = repository;
      _immutable(this, obj).set("id").set("data");
    }

    return Blob;

  })();

  Gitteh.Tag = Tag = (function() {
    /*
    	Git tags are similar to references, and indeed "lightweight" Git tags are 
    	actually implemented as :class:`gitteh::Reference` with a name prefix
    	of "tags/". When additional metadata is needed (message/name/email/GPG
    	signature), a proper heavyweight Tag object is used.
    
    	Properties:
    
    	* **id**: *(String)* OID of this Tag.
    	* **name**: *(String)*
    	* **message**: *(String)*
    	* **tagger**: *(Signature)*
    	* **targetId**: *(String)* OID this Tag points to
    	* **type**: *(String)* the type of object this Tag points to.
    */
    function Tag(repository, obj) {
      this.repository = repository;
      obj.tagger = new Signature(obj.tagger);
      _immutable(this, obj).set("id").set("name").set("message").set("tagger").set("target", "targetId").set("type");
    }

    Tag.prototype.target = function(cb) {
      /*
      		Convenience method to get the object this Tag points to. Shorthand for
      		calling :func:`gitteh::Repository.object` with this *targetId*
      */      return this.repository.object(this.targetId, this.type, cb);
    };

    return Tag;

  })();

  Gitteh.Remote = Remote = (function() {
    /*
    	Remotes designate the location and rules of remote Git repositories. Remotes
    	can be obtained by using :func:`gitteh::Repository.remote`
    
    	Properties:
    
    	* **connected**: *(Boolean)* true if there is an active connection to the Remotes' endpoint.
    	* **name**: *(String)*
    	* **url**: *(String)* address of Remotes' endpoint
    	* **fetchSpec**: (:class:`gitteh::Refspec`) Refspec used when fetching from Remote
    	* **pushSpec**: (:class:`gitteh::Refspec`) Refspec used when pushing to Remote
    	* **HEAD**: *(String)* the remote HEAD reference name (only set after connected to Remote)
    	* **refs**: *(String[])* names of references on remote (only set after connected to Remote)
    */
    function Remote(repository, nativeRemote) {
      var fetchSpec, pushSpec, _priv;
      this.repository = repository;
      _priv = _createPrivate(this);
      _priv["native"] = nativeRemote;
      _priv.connected = false;
      if (!(nativeRemote instanceof NativeRemote)) {
        throw new Error("Don't construct me, see Repository.remote()");
      }
      Object.defineProperty(this, "connected", {
        get: function() {
          return _priv.connected;
        },
        enumerable: true,
        configurable: false
      });
      _immutable(this, nativeRemote).set("name").set("url");
      fetchSpec = new Refspec(nativeRemote.fetchSpec.src, nativeRemote.fetchSpec.dst);
      pushSpec = new Refspec(nativeRemote.pushSpec.src, nativeRemote.pushSpec.dst);
      _immutable(this, {
        fetchSpec: fetchSpec,
        pushSpec: pushSpec
      }).set("fetchSpec").set("pushSpec");
    }

    Remote.prototype.connect = function() {
      /*
      		Opens a connection to the :class:`gitteh::Remote` endpoint. This is
      		needed before :func:`gitteh::Remote.fetch` or :func:`gitteh::Remote.push`
      		can be called. `direction` must be supplied as either "push" or "fetch"
      		and `cb` will be called once Remote is connected.
      */
      var cb, dir, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        dir: {
          type: "remoteDir"
        },
        cb: {
          type: "function"
        }
      }), dir = _ref[0], cb = _ref[1];
      dir = dir === "push" ? bindings.GIT_DIRECTION_PUSH : bindings.GIT_DIRECTION_FETCH;
      return _priv["native"].connect(dir, _wrapCallback(cb, function(refs) {
        var headOid, headRef, oid, ref, refNames;
        refNames = Object.keys(refs);
        headOid = refs["HEAD"];
        for (ref in refs) {
          oid = refs[ref];
          if (ref === "HEAD") continue;
          if (oid === headOid) {
            headRef = _this.fetchSpec.transformTo(ref);
            _immutable(_this, {
              headRef: headRef
            }).set("headRef", "HEAD");
            break;
          }
        }
        _immutable(_this, {
          refNames: refNames
        }).set("refNames", "refs");
        _priv.connected = true;
        return cb();
      }));
    };

    Remote.prototype.fetch = function() {
      /*
      		Fetches Git objects from remote that do not exist locally. `progressCb`
      		will be called regularly to notify callers of fetch progress and `cb`
      		will be called once fetch has completed.
      */
      var cb, progressCb, update, updateTimer, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      if (!this.connected) throw new Error("Remote isn't connected.");
      _ref = args({
        progressCb: {
          type: "function"
        },
        cb: {
          type: "function"
        }
      }), progressCb = _ref[0], cb = _ref[1];
      updateTimer = null;
      update = function() {
        var bytes, done, total, _ref2;
        _ref2 = _priv["native"].stats, bytes = _ref2.bytes, total = _ref2.total, done = _ref2.done;
        progressCb(bytes, total, done);
        return updateTimer = setTimeout(update, 500);
      };
      setTimeout(update, 500);
      return _priv["native"].download(function(err) {
        clearTimeout(updateTimer);
        if (err != null) return cb(err);
        return _priv["native"].updateTips(_wrapCallback(cb, function() {
          return cb();
        }));
      });
    };

    return Remote;

  })();

  Gitteh.Index = Index = (function() {
    /*
    	The Git index is used to stage changed files before they are written to the 
    	repository proper. Bindings for the Index are currently minimal, expect
    	this to change in a newer version.
    */
    var write;

    function Index(nativeIndex) {
      var _priv;
      _priv = _createPrivate(this);
      _priv["native"] = nativeIndex;
    }

    Index.prototype.readTree = function() {
      /*
      		Updates the Git index to reflect the state of provided
      		:class:`gitteh::Tree` (using OID passed from `id` parameter). `cb` will
      		be called once index update has completed.
      */
      var cb, id, _priv, _ref;
      _priv = _getPrivate(this);
      _ref = args({
        id: {
          type: "oid"
        },
        cb: {
          type: "function"
        }
      }), id = _ref[0], cb = _ref[1];
      return _priv["native"].readTree(id, cb);
    };

    Index.prototype.write = function() {
      /*
      		Synchronizes the in-memory Git index with the indexfile located in
      		repository, and calls `cb` once synchronization is complete.
      */
      var cb, _priv;
      _priv = _getPrivate(this);
      cb = args({
        cb: {
          type: "function"
        }
      })[0];
      return _priv["native"].write(cb);
    };

    return Index;

  })();

  Gitteh.Reference = Reference = (function() {
    /*
    	A Reference is a named pointer to a :class:`gitteh::Commit` object. That is,
    	refs are the DNS of Git-land. References can either be direct or symbolic.
    	Direct references point to the object id of a commit. Symbolic refs point
    	to other references.
    
    	References can be obtained using :func:`gitteh::Repository.reference` and
    	created using :func:`gitteh::Repository.createReference`.
    
    	Properties:
    
    	* **name**: *(String)*
    	* **direct**: *(Boolean)* true if Reference points directly to an object id.
    	* **packed**: *(Boolean)* true if Reference is in a packfile
    	* **target**: *(String)* object id reference points to, or another reference
    	name if not a direct reference.
    	* **repository**: (:class:`gitteh::Repository`) owner of this Reference.
    */
    function Reference(repo, nativeRef) {
      var _priv;
      _priv = _createPrivate(this);
      _priv["native"] = nativeRef;
      _immutable(this, nativeRef).set("name").set("direct").set("packed").set("target");
      _immutable(this, {
        repo: repo
      }).set("repo", "repository");
    }

    return Reference;

  })();

  Gitteh.Repository = Repository = (function() {
    /*
    	Represents a local Git repository that has been opened by Gitteh. Objects
    	such as :class:`gitteh::Commit`, :class:`gitteh.Tree` and 
    	:class:`gitteh.Reference` can be obtained and created from a Repository.
    	
    	Repositories can be *bare* - they will not have a working directory, in this
    	case the contents of what is usually in a .git subdirectory will be in the
    	top level.
    
    	Properties:
    
    	* **bare**: *(Boolean)* true if this repository is bare.
    	* **path**: *(String)* location of the Git metadata directory
    	* **workingDirectory**: *(String)* location of the working directory, if applicable (non-bare repository)
    	* **remotes**: *(String[])*  names of remotes configured for this repository
    	* **references**: *(String[])* names of references contained in this repository.
    	* **index**: (:class:`gitteh::Index`) The Git index for this repository.
    */
    function Repository(nativeRepo) {
      var index, _priv;
      if (!(nativeRepo instanceof NativeRepository)) {
        throw new Error("Don't construct me, see gitteh.(open|init)Repository");
      }
      _priv = _createPrivate(this);
      _priv["native"] = nativeRepo;
      _immutable(this, nativeRepo).set("bare").set("path").set("workDir", "workingDirectory").set("remotes").set("references").set("submodules");
      index = new Index(nativeRepo.index);
      _immutable(this, {
        index: index
      }).set("index");
    }

    Repository.prototype.exists = function(oid, cb) {
      /*
      		Checks if an object with given `oid` exists. Calls `cb` with result.
      */
      var _priv, _ref;
      _priv = _getPrivate(this);
      _ref = args({
        oid: {
          type: "oid"
        },
        cb: {
          type: "function"
        }
      }), oid = _ref[0], cb = _ref[1];
      return _priv["native"].exists(oid, cb);
    };

    Repository.prototype.object = function() {
      /*
      		Fetches an object with given `oid` and returns the result to provided 
      		`cb`. The object returned will be a Gitteh wrapper corresponding to the
      		type of Git object fetched.
      
      		Alternatively, objects with an expected type can be fetched using the
      		:func:`gitteh::Repository.blob` :func:`gitteh::Repository.commit`
      		:func:`gitteh::Repository.tag` :func:`gitteh::Repository.tree` and
      		:func:`gitteh::Repository.reference` methods.
      */
      var cb, oid, type, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        oid: {
          type: "oid"
        },
        type: {
          type: "objectType",
          "default": "any"
        },
        cb: {
          type: "function"
        }
      }), oid = _ref[0], type = _ref[1], cb = _ref[2];
      return _priv["native"].object(oid, type, _wrapCallback(cb, function(object) {
        var clazz;
        clazz = (function() {
          switch (object._type) {
            case types.commit:
              return Commit;
            case types.tree:
              return Tree;
            case types.blob:
              return Blob;
            case types.tag:
              return Tag;
            default:
              return;
          }
        })();
        if (clazz === void 0) return cb(new TypeError("Unexpected object type"));
        return cb(null, new clazz(_this, object));
      }));
    };

    Repository.prototype.blob = function(oid, cb) {
      /*
      		Fetches a :class:`gitteh::Blob` object with given `oid` from the
      		repository. This is a stricter variant of :func:`gitteh::Repository.object`
      		- an error will be thrown if requested object isnt a Blob.
      */      return this.object(oid, "blob", cb);
    };

    Repository.prototype.commit = function(oid, cb) {
      /*
      		Fetches a :class:`gitteh::Commit` object with given `oid` from the
      		repository. This is a stricter variant of :func:`gitteh::Repository.object`
      		- an error will be thrown if requested object isnt a Commit.
      */      return this.object(oid, "commit", cb);
    };

    Repository.prototype.tag = function(oid, cb) {
      /*
      		Fetches a :class:`gitteh::Tag` object with given `oid` from the
      		repository. This is a stricter variant of :func:`gitteh::Repository.object`
      		- an error will be thrown if requested object isnt a Tag.
      */      return this.object(oid, "tag", cb);
    };

    Repository.prototype.tree = function(oid, cb) {
      /*
      		Fetches a :class:`gitteh::Tree` object with given `oid` from the
      		repository. This is a stricter variant of :func:`gitteh::Repository.object`
      		- an error will be thrown if requested object isnt a Tree.
      */      return this.object(oid, "tree", cb);
    };

    Repository.prototype.reference = function() {
      /*
      		Fetches a :class:`gitteh::Reference` object with given `oid` from the
      		repository. This is a stricter variant of :func:`gitteh::Repository.object`
      		- an error will be thrown if requested object isnt a Reference.
      */
      var cb, name, resolve, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        name: {
          type: "string"
        },
        resolve: {
          type: "bool",
          "default": false
        },
        cb: {
          type: "function"
        }
      }), name = _ref[0], resolve = _ref[1], cb = _ref[2];
      return _priv["native"].reference(name, resolve, _wrapCallback(cb, function(ref) {
        return cb(null, new Reference(_this, ref));
      }));
    };

    Repository.prototype.createReference = function() {
      /*
      		Creates a new reference, which can either by direct or symbolic.
      
      		Parameters:
      
      		* **name**: *(String)*
      		* **target**: *(String)* reference/oid targetted by the new reference.
      		* **[force=false]**: *(String)* force creation of this reference, destroying the reference with same name, if it exists.
      		* **cb**: *(String)* called when reference has been created.
      */
      var cb, fn, force, name, target, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        name: {
          type: "string"
        },
        target: {
          type: "string"
        },
        force: {
          type: "bool",
          "default": false
        },
        cb: {
          type: "function"
        }
      }), name = _ref[0], target = _ref[1], force = _ref[2], cb = _ref[3];
      fn = "createSymReference";
      if (target.length === 40 && args.oidRegex.test(target)) {
        fn = "createOidReference";
      }
      return _priv["native"][fn](name, target, force, _wrapCallback(cb, function(ref) {
        return cb(null, new Reference(_this, ref));
      }));
    };

    Repository.prototype.remote = function() {
      /**
      		Loads a remote with given `name`.
      */
      var cb, name, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        name: {
          type: "string"
        },
        cb: {
          type: "function"
        }
      }), name = _ref[0], cb = _ref[1];
      return _priv["native"].remote(name, _wrapCallback(cb, function(remote) {
        return cb(null, new Remote(_this, remote));
      }));
    };

    Repository.prototype.createRemote = function() {
      /**
      		Create a new :class:`gitteh::Remote` for this repository with the given
      		`name` and `url`. Calls `cb` when the operation has completed.
      */
      var cb, name, url, _priv, _ref,
        _this = this;
      _priv = _getPrivate(this);
      _ref = args({
        name: {
          type: "string"
        },
        url: {
          type: "string"
        },
        cb: {
          type: "function"
        }
      }), name = _ref[0], url = _ref[1], cb = _ref[2];
      return _priv["native"].createRemote(name, url, _wrapCallback(cb, function(remote) {
        return cb(null, new Remote(_this, remote));
      }));
    };

    return Repository;

  })();

  /**
   * Alias of {@link #reference}.
   * @param {String} oid id of reference to be fetched.
   * @param {Function} cb called when reference has been fetched.
   * @see #reference
  */

  Repository.prototype.ref = Repository.prototype.reference;

  Gitteh.openRepository = function() {
    /*
    	Opens a local Git repository with the given `path`. Calls `cb` once
    	a :class:`gitteh::Repository` has been opened.
    */
    var cb, path, _ref;
    _ref = args({
      path: {
        type: "string"
      },
      cb: {
        type: "function"
      }
    }), path = _ref[0], cb = _ref[1];
    return bindings.openRepository(path, _wrapCallback(cb, function(repo) {
      return cb(null, new Repository(repo));
    }));
  };

Gitteh.openIndex = function() {
    /*
        Opens a local Git repository index. Calls `cb` once
        a :class:`gitteh::Index` has been opened.
    */
    var cb, path, _ref;
    _ref = args({
      repo: {
        type: "repository"
      },
      cb: {
        type: "function"
      }
    }), repo = _ref[0], cb = _ref[1];
    return bindings.openIndex(repo, _wrapCallback(cb, function(repo) {
      return cb(null, new Index(repo));
    }));
  };



  Gitteh.initRepository = function() {
    /*
    	Creates a new Git repository at `path`, and calls `cb` providing the new 
    	:class:`gitteh::Repository` when operation has completed.
    
    	If `bare` is specified as true the repository will be created without a
    	working directory. For more information see (TODO: link to page describing
    	bare repositories.)
    */
    var bare, cb, path, _ref;
    _ref = args({
      path: {
        type: "string"
      },
      bare: {
        type: "bool",
        "default": false
      },
      cb: {
        type: "function"
      }
    }), path = _ref[0], bare = _ref[1], cb = _ref[2];
    return bindings.initRepository(path, bare, _wrapCallback(cb, function(repo) {
      return cb(null, new Repository(repo));
    }));
  };

  Gitteh.clone = function() {
    /*
    	Provides high level "git clone" functionality.
    
    	Creates a new repository on the local filesystem at `path`, creates a 
    	default "origin" :class:`gitteh::Remote` with the provided `url`, fetches
    	the references from the remote, checks out the master branch into the
    	working directory.
    
    	Currently, libgit2 only supports HTTP/Git protocols - there is no support
    	for git+ssh yet.
    */
    var cb, emitter, path, url, _ref;
    _ref = args({
      url: {
        type: "string"
      },
      path: {
        type: "string"
      }
    }), url = _ref[0], path = _ref[1], cb = _ref[2];
    emitter = new EventEmitter;
    async.waterfall([
      function(cb) {
        return Gitteh.initRepository(path, false, cb);
      }, function(repo, cb) {
        return repo.createRemote("origin", url, _wrapCallback(cb, function(remote) {
          return cb(null, repo, remote);
        }));
      }, function(repo, remote, cb) {
        return remote.connect("fetch", _wrapCallback(cb, function() {
          return cb(null, repo, remote);
        }));
      }, function(repo, remote, cb) {
        var emitProgress;
        emitProgress = function(bytes, done, complete) {
          return emitter.emit("status", bytes, done, complete);
        };
        return remote.fetch(emitProgress, _wrapCallback(cb, function() {
          return cb(null, repo, remote);
        }));
      }, function(repo, remote, cb) {
        return repo.ref(remote.HEAD, true, _wrapCallback(cb, function(ref) {
          return cb(null, repo, remote, ref);
        }));
      }, function(repo, remote, headRef, cb) {
        var refName;
        refName = remote.fetchSpec.transformFrom(remote.HEAD);
        return repo.createReference(refName, headRef.target, _wrapCallback(cb, function() {
          return cb(null, repo, remote, headRef);
        }));
      }, function(repo, remote, headRef, cb) {
        return repo.commit(headRef.target, _wrapCallback(cb, function(commit) {
          return cb(null, repo, remote, commit);
        }));
      }, function(repo, remote, headCommit, cb) {
        return headCommit.tree(_wrapCallback(cb, function(tree) {
          return cb(null, repo, remote, tree);
        }));
      }, function(repo, remote, headTree, cb) {
        var checkoutTree, handleEntry;
        handleEntry = function(dest, entry, cb) {
          var subPath;
          if (entry.type === "tree") {
            subPath = _path.join(dest, entry.name);
            return async.series([
              function(cb) {
                return fs.mkdir(subPath, cb);
              }, function(cb) {
                return repo.tree(entry.id, _wrapCallback(cb, function(subtree) {
                  return checkoutTree(subtree, subPath, cb);
                }));
              }
            ], cb);
          } else if (entry.type === "blob") {
            return repo.blob(entry.id, _wrapCallback(cb, function(blob) {
              var file;
              file = fs.createWriteStream(_path.join(dest, entry.name), {
                mode: entry.attributes
              });
              file.write(blob.data);
              file.end();
              return cb();
            }));
          } else {
            return cb();
          }
        };
        checkoutTree = function(tree, dest, cb) {
          return async.forEach(tree.entries, handleEntry.bind(null, dest), cb);
        };
        return checkoutTree(headTree, repo.workingDirectory, _wrapCallback(cb, function() {
          return cb(null, repo, remote, headTree);
        }));
      }, function(repo, remote, headTree, cb) {
        return repo.index.readTree(headTree.id, _wrapCallback(cb, function() {
          return cb(null, repo, remote);
        }));
      }, function(repo, remote, cb) {
        return repo.index.write(_wrapCallback(cb, function() {
          return cb(null, repo, remote);
        }));
      }
    ], function(err, repo) {
      if (err != null) return emitter.emit("error", err);
      return emitter.emit("complete", repo);
    });
    return emitter;
  };

}).call(this);
