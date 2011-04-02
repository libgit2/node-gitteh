/**
 * @fileOverview This is just an empty file containing JSDoc markup to generate nice documentation for stuff that is actually implemented in C++.
 * @author Sam Day
 * @version 0.1.0
 */

/**
 * Represents an open Git repository. The majority of the functionality offered 
 * by Gitteh is contained in here.
 * @class
 */
var Repository = function() {};

/**
 * This is the namespace of the Gitteh module.
 * @namespace
 * @example
 * var gitteh = require("gitteh");
 */
var Gitteh = { };

/**
 * Opens a Git repository at the location provided. You can either provide a
 * path to a Git repository, which will open the repository "bare" (no working
 * directory), or you can provide an object containing paths to the various 
 * critical Git directories needed for Git to operate.
 * @param {String|Object} path
 * @param {String} path.gitDirectory The path to the Git repository folder (e.g /foo/.git)
 * @param {String} [path.workTree] The path to the Git working directory (e.g /foo). Omitting this will assume the repository is bare.
 * @param {String} [path.objectDirectory] The path to the ODB directory (e.g /foo/.git/objects). Omitting this will default to path.gitDirectory + "/objects"
 * @param {String} [path.indexFile] The path to the repository index file (e.g /foo/.git/index). Omitting this will default to path.gitDirectory + "/index"
 * @param {Function} [callback] If provided, the repository will be opened asynchronously, and the provided callback will be fired when the repository has been opened.
 * @returns {Repository}
 * @throws If the path provided is incorrect, or the repository is corrupt in some way. 
 */
Gitteh.openRepository = function(path) { };

/**
 * Creates a new Git repository at the provided path. The repository will be
 * created at the path provided, unless bare is set to true, in which case the 
 * repository will be initialized in a <i>.git</i> directory inside the path
 * provided.
 * @param {String} path The path to initialize the new Git repository in.
 * @param {Boolean} [bare] Initialize the repository as bare or "checked out".
 * @param {Function} [callback] If provided, the repository will be created asynchronously. The provided callback will be fired when the repository has been created.
 */
Gitteh.initRepository = function(path, bare, callback) { };

/**
 * Contains the various error constants found throughout Gitteh.
 * @namespace
 */
Gitteh.error = { };

// Ffs. There's supposed to be a JSDoc "metatag" that allows me to define one
// set of tags and it will apply to everything that follows. It doesn't work.
// Hence the bullshit here.
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_SUCCESS = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ERROR = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ENOTOID = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ENOTFOUND = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ENOMEM = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EOSERR = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EOBJTYPE = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EOBJCORRUPTED = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ENOTAREPO = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EINVALIDTYPE = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EMISSINGOBJDATA = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EPACKCORRUPTED = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EFLOCKFAIL = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EZLIB = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EBUSY = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EBAREINDEX = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EINVALIDREFNAME = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EREFCORRUPTED = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ETOONESTEDSYMREF = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EPACKEDREFSCORRUPTED = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EINVALIDPATH = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EREVWALKOVER = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_EINVALIDREFSTATE = undefined;
/**
 * @static
 * @constant
 */
Gitteh.error.GIT_ENOTIMPLEMENTED = undefined;

/**
 * Retrieves a {@link Commit} from the repository with the given sha1 hash id.
 * @param {String} sha1 The sha1 hash ID of the commit.
 * @param {Function} [callback] If callback is provided, Vommit will be retrieved asynchronously and passed to to the callback provided.
 * @return {Commit}
 * @throws If commit wasn't found, or there is some other error. 
 */
Repository.prototype.getCommit = function(sha1, callback) {};

/**
 * Creates a new Commit and immediately stores it in the Repository.
 * @param {Object} data The data for the commit. Should be the same properties that a {@link Commit} contains.
 * @param {Function} [callback] If provided, the Commit will be created asynchronously, and the newly created Commit object returned to the callback.
 * @throws If Commit couldn't be created.
 */
Repository.prototype.createCommit = function(data, callback) {};

/**
 * Retrieves a {@link Tree} from the repository with the given sha1 hash id.
 * @param {String} sha1 The sha1 hash ID of the tree.
 * @param {Function} [callback] If a callback is provided, the Tree will be retrieved asynchronously and returned via the callback given.
 * @return {Tree}
 * @throws If Tree wasn't found, or some other error occurs. 
 */
Repository.prototype.getTree = function(sha1, callback) {};

/**
 * Retrieves a {@link Reference} from the Repository with the given name.
 * @param {String} name The name of the reference, for example: refs/heads/master. You may also retrieve <i>HEAD</i> using this method.
 * @param {Function} [callback] If provided, the reference will be obtained asychronously, and provided to the callback.
 * @return {Reference}
 * @throws If Reference does not exist, or some other error occured. 
 */
Repository.prototype.getReference = function(name, callback) {};

/**
 * 
 * @param {Function} [callback]
 */
Repository.prototype.createWalker = function(callback) {};

/** 
 * @class
 * @property {String} id The id of the commit. This property is read-only.
 * @property {String} message The commit message.
 * @property {Signature} author The author of the commit.
 * @property {Signature} committer The commit committer.
 * @property {Tree} tree The tree associated with the commit.
 * @property {Commit[]} parents The parents of this commit, if any. 
 */
var Commit = function() {};

/**
 * Saves the commit to the repository. This will update the {@link id} property
 * when successful.
 * @param {Function} [callback] If provided, commit will be saved asynchronously, and the result of the save will be passed to the callback.
 * @return {Boolean} true if successful.
 * @throws If the commit failed to save.
 */
Commit.prototype.save = function() {}; 

/**
 * A Tree represents the snapshot of a repo for a specific {@link Commit}.
 * @class
 * @property {String} id The sha1 hash of this Tree.
 * @property {TreeEntry[]} entries The entries contained in this Tree.
 */
var Tree = function() {};

/**
 * Saves the Tree to the Repository. <b>THIS IS UNIMPLEMENTED CURRENTLY</b>
 * @throws If called, because it's currently unimplemented.
 */
Tree.prototype.save = function() {};

/**
 * A single entry in a {@link Tree}. Note this isn't a "real" class, as there's
 * no constructor. Tree entries are just plain old Javascript objects. This is  
 * just here for reference.
 * @class
 * @property {String} id SHA1 hash of the {@link Blob} this entry points to.
 * @property {String} name Filename of the entry.
 * @property {Integer} attributes File attributes for the entry.
 */
var TreeEntry = function() {};

/**
 * A reference serves as a pointer of sorts to a specific Commit. References can
 * either be <i>direct</i> or <i>symbolic</i>. Direct references point to a commit
 * OID, symbolic references point to other references (which may also be symbolic).
 * References can be obtained through {@link Repository#getReference}.
 * @class
 * @property {String} name The name of the reference - e.g refs/heads/master
 * @property {String} type The type of reference this is. Can be either "oid" or "symbolic". CANNOT be changed.
 * @property {String} target Either the SHA1 hash of the {@link Commit} being pointed to by this Reference, if it's type is "oid". If it's a symbolic Reference, this will point to another reference name. 
 */ 
var Reference = function() {};

/**
 * Renames this Reference. Name of the new Reference must not already exist in the Repository.
 * @param {String} newName The new name.
 * @param {Function} [callback] If provided, the rename will occur asynchronously, and the callback will be fired when rename has completed.
 * @return {Boolean} true if successful.
 * @throws If an error occurred during renaming process.
 */
Reference.prototype.rename = function(newName, callback) { };

/**
 * Deletes the Reference permanently. Once deleted, the Reference will no longer
 * be accessible from the Repository. It will be deleted from the filesystem
 * also. If this Reference is involved in any symbolic link chains, they will
 * become invalid. After calling this, any further calls on the Reference will
 * throw an Error.
 * @name Reference#delete
 * @function
 * @param {Function} [callback] If provided, the deletion will occur asynchronously and return the status of the delete to the callback.
 * @return {Boolean} true if successful
 * @throws If an error occurred while deleting the Reference.
 */
// Used @name tag because Eclipse linter doesn't like method name being "delete".
//Reference.prototype.delete = function(callback) { };

/**
 * Resolves this Reference to a direct oid Reference. This means it will follow
 * symbolic links until it arrives at the direct link pointing to a {@ link Commit}.
 * Note that if you call this on a Reference that is already pointing to a Commit, it
 * will just return the Reference.
 * @param {Function} [callback] When given, the resolution will be done asynchronously and the resulting Reference passed to the callback.
 * @return {Reference} The result of the resolution.
 * @throws If an error occurred while resolving the Reference.
 */
Reference.prototype.resolve = function(callback) { };

/**
 * Sets the target of this Reference. This change is immediate. <b>Note:</b> You
 * can't change the type of a Reference. For example, if this Reference is an
 * OID reference, you can't set the target to another Reference. If you need to
 * do this, delete the original ref, and recreate it as the other type. 
 * @param {String} newTarget either the oid, or name of another Reference. 
 * @param {Function} [callback] If provided, the Reference will be re-targeted asynchronously. The callback will be notified when this occurs.
 * @return {Boolean} true if successful.
 * @throws If an error occurred while setting target.
 */
Reference.prototype.setTarget = function(newTarget, callback) { };

/**
 * @static
 */
Gitteh.GIT_REF_OID = undefined;
/**
 * @static
 */
Gitteh.GIT_REF_SYMBOLIC = undefined;
/**
 * @static
 */
Gitteh.GIT_REF_PACKED = undefined;
/**
 * @static
 */
Gitteh.GIT_REF_LISTALL = undefined;

/**
 * A Blob is a binary store of a specific file that has been committed into a
 * Repository at some point.
 * @class
 * @property {String} id The SHA1 id of the Blob.
 * @property {Buffer} data The data this Blob contains.
 */
var Blob = function() {};

/**
 * Saves this blob to Repository and updates its id.
 * @param {Function} [callback] If provided, the save will be performed asynchronously, and the result passed to the callback.
 * @return {Boolean} true if successful.
 * @throws If save was not successful.
 */
Blob.prototype.save = function(callback) {};

/**
 * A RevisionWalker provides a clean interface to walk the commit history of a
 * {@link Repository}. A RevisionWalker may be obtained via {@link Repository#createWalker}
 * @class 
 */
var RevisionWalker = function() {};

/**
 * Errors thrown by Gitteh will generally include additional data if it's a Git
 * related issue. Note that there is no actual Error type called GitError, this 
 * is just for illustrative purposes only.
 * @class
 * @extends Error
 * @property {Integer} gitError The error number. The possible errors are detailed in {@link Gitteh.error}.
 * @property {String} gitErrorStr A short message explaining the error.
 */
var GitError = function() {};