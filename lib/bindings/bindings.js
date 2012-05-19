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
Gitteh.error.GIT_OK = undefined;
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
 * Creates a new {@link Tree} in the Repository with the given data. <b>NOTE:</b>
 * this is currently unimplemented.
 * @param {Object} data
 * @param {Function} [callback]
 */
Repository.prototype.createTree = function(data, callback) {};

/**
 * Retrieves a {@link Reference} from the Repository with the given name.
 * @param {String} name The name of the reference, for example: refs/heads/master. You may also retrieve <i>HEAD</i> using this method.
 * @param {Function} [callback] If provided, the reference will be obtained asychronously, and provided to the callback.
 * @return {Reference}
 * @throws If Reference does not exist, or some other error occured. 
 */
Repository.prototype.getReference = function(name, callback) {};

/**
 * Creates a new OID (direct) {@link Reference} in the Repository.
 * @param {String} name The name of the Reference to be created.
 * @param {String} oid The ID of the {@link Commit} being pointed to.
 * @param {Function} [callback] When provided, the Reference shall be created asynchronously. The callback will be notified when creation is complete.
 * @returns {Reference}
 * @throws If there was an error creating the Reference.
 */
Repository.prototype.createOidReference = function(name, oid, callback) {};

/**
 * Creates a new symbolic (indirect) {@link Reference} in the Repository.
 * @param {String} name The name of the Reference to be created.
 * @param {String} target The name of the Reference being pointed to.
 * @param {Function} [callback] When provided, the Reference shall be created asynchronously. The callback will be notified when creation is complete.
 * @returns {Reference}
 * @throws If there was an error creating the Reference.
 */
Repository.prototype.createSymbolicReference = function(name, target, callback) {};

/**
 * Requests a list of References from the Repository. This will return a list of
 * Strings, each String is a name of a reference in the Repository.
 * @param {Integer} [flags] If provided, the list will be filtered based on input. Valid values are {@link Gitteh.GIT_REF_OID}, {@link Gitteh.GIT_REF_SYMBOLIC}, {@link Gitteh.GIT_REF_PACKED}, and {@link Gitteh.GIT_REF_LISTALL}
 * @param {Function} [callback] If provided, the list will be built asynchronously, and the results provided to the callback.
 * @returns {String[]}
 * @throws If an error occurs during the listing process.
 * 
 * @see Gitteh.GIT_REF_OID
 * @see Gitteh.GIT_REF_SYMBOLIC
 * @see Gitteh.GIT_REF_PACKED
 * @see Gitteh.GIT_REF_LISTALL
 */
Repository.prototype.listReferences = function(flags, callback) {};

/**
 * Orders the Repository to pack all loose {@link Reference}s. This method will
 * ensure any currently active {@link Reference}s remain valid (even though they
 * have now been moved inside the packed-references file).  
 * @param {Function} [callback] If provided, the packing process will be performed asynchronously and the callback notified of the result.
 * @returns {Boolean} true if successful
 * @throws If an error occurs during the packing process.
 */
Repository.prototype.packReferences = function(callback) {};

/**
 * Creates a new {@link RevisionWalker}.
 * @param {Function} [callback] If provided, the walker will be created asychronously.
 * @returns {RevisionWalker}
 * @throws If an error occurs.	
 */
Repository.prototype.createWalker = function(callback) {};

/**
 * Loads the Index for the Repository.
 * @param {Function} [callback] If provided, the Index will be loaded asynchronously, and the result handed to the callback.
 * @returns {Index}
 * @throws If an error occurred while loading Index.
 */
Repository.prototype.getIndex = function(callback) {};

/**
 * Creates a new {@link Tag} with the provided data.  
 * @param {Object} data The data to be stored in the Tag. The fields in this object should be the same as the fields in a {@link Tag}.
 * @param {Function} [callback] If provided, the tag will be created asynchronously and the newly created Tag provided to the callback.
 * @return {Tag}
 * @throws If an error occurred whilst creating the Tag.
 */
Repository.prototype.createTag = function(data, callback) {};

/**
 * Gets a {@link Tag} from the Repository with the given sha1 hash id. 
 * @param {String} sha1 the sha1 hash ID of the Tag.
 * @param {Function} [callback] When provided, the Tag will be retrieved asynchronously and results handed to the callback.
 * @return {Tag}
 * @throws If an error occurred while loading the Tag.
 */	
Repository.prototype.getTag = function(sha1, callback) {};

/**
 * Creates a new {@link Blob} in the Repository.
 * @param {Object} data The data for the Blob. Valid fields in this data are the same as the fields in {@link Blob}
 * @param {Function} [callback] If provided, the Blob will be created asynchronously, and the results passed to the callback.
 * @returns {Blob}
 * @throws If an error occurs while creating the Blob.
 */
Repository.prototype.createBlob = function(data, callback) {};

/**
 * Retrieves a Blob from the Repository with given id.
 * @param {String} sha1 The sha1 hash id of the blob.
 * @param {Function} [callback] If provided, the Blob will be retrieved asynchronously, and provided to the callback.
 * @returns {Blob}
 * @throws If an error occurs while loading the Blob.
 */
Repository.prototype.getBlob = function(sha1, callback) {};

/**
 * Checks if the Repository contains an object with the specified sha1 hash.
 * @param {String} sha1 SHA1 hash id of the object being searched for.
 * @param {Function} [callback] If provided, the search will be performed asynchronously, with the result passed back to callback.
 * @param {Boolean} true if object exists, false if not.
 * @throws If an error occurs.
 */
Repository.prototype.exists = function(sha1, callback) {};

/** 
 * @class
 * @property {String} id The id of the commit. This property is read-only.
 * @property {String} message The commit message.
 * @property {Signature} author The author of the commit.
 * @property {Signature} committer The commit committer.
 * @property {Tree} tree The tree associated with the commit.
 * @property {Commit[]} parents The parents of this commit, if any.
 * 
 * @see Repository#getCommit
 * @see Repository#createCommit
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
 * 
 * @see Repository#getTree
 * @see Repository#createTree
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
 * @class
 * @property {String} name The name of the reference - e.g refs/heads/master
 * @property {String} type The type of reference this is. Can be either "oid" or "symbolic". CANNOT be changed.
 * @property {String} target Either the SHA1 hash of the {@link Commit} being pointed to by this Reference, if it's type is "oid". If it's a symbolic Reference, this will point to another reference name.
 * 
 * @see Repository#getReference
 * @see Repository#createOidReference
 * @see Repository#createSymbolicReference
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
 * List OID (direct) References
 * @static
 */
Gitteh.GIT_REF_OID = undefined;
/**
 * List symbolic (indirect) References.
 * @static
 */
Gitteh.GIT_REF_SYMBOLIC = undefined;
/**
 * Include packed References in list.
 * @static
 */
Gitteh.GIT_REF_PACKED = undefined;
/**
 * List all References in the Repository.
 * @static
 */
Gitteh.GIT_REF_LISTALL = undefined;

/**
 * A Blob is a binary store of a specific file that has been committed into a
 * Repository at some point.
 * @class
 * @property {String} id The SHA1 id of the Blob.
 * @property {Buffer} data The data this Blob contains.
 * 
 * @see Repository#createBlob
 * @see Repository#getBlob
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
 * The Index represents the cache Git stores for your working directory. When
 * you "stage" a file in Git, this is basically just a command that will add 
 * or update your file in the Git index. Running a Commit in Git is essentially
 * just writing a Commit with a Tree that matches an Index. <b>Note:</b> There
 * will be a method soon to create a Tree from an opened Index, this feature has
 * not yet been implemented by libgit2, as soon as it is it will be in Gitteh.
 * @class
 * @property {Integer} entryCount <i>Read-only</i> How many entries are contained in the Index.
 * 
 * @see Repository#getIndex
 */
var Index = function() {};

/**
 * Retrieves an {@link IndexEntry} from the Index.
 * @param {Integer} index The index to retrieve. 
 * @param {Function} [callback] When provided, IndexEntry will be retrieved asynchronously and provided back to the callback.
 * @return {IndexEntry}
 * @throws If index is out of bounds, or some other error occurs.
 */
Index.prototype.getEntry = function(index, callback) {};

/**
 * Searches the Index for an entry that matches the given filename.
 * @param {String} [fileName] The filename of entry to search for.
 * @param {Function} [callback] If provided, entry will be searched for in a non-blocking manner, and results returned to the callback.
 * @return {IndexEntry}
 * @throws If entry could not be found, or some other error occurs.
 */
Index.prototype.findEntry = function(fileName, callback) {};

/**
 * Adds an entry to the Index for the given fileName. This file should already 
 * exist in the working directory of the Repository. <b>Note:</b> this function
 * will fail if called on a Index for a bare repository, as it has no working
 * directory.
 * @param {String} fileName Name of the file to update index entry for.
 * @param {Function} [callback] When provided, the callback will be notified when the entry has been added asynchronously.
 * @returns {Boolean} true if successful
 * @throws If there was an error adding the entry.
 */
Index.prototype.addEntry = function(fileName, callback) {};

/**
 * Changes to an Index will not be updated on the actual index file until this 
 * method is called.
 * @param {Function} [callback] When provided, the writeback will be performed in an asynchronous fashion, and the result will be passed back to the callback.
 * @returns {Boolean} true if successful.
 * @throws If there was an error writing the Index.
 */
Index.prototype.write = function(callback) {};

/**
 * Represents a single entry in the Git {@link Index}. <b>Note:</b> While the 
 * constructor for this class <i>is</i> being exposed, it is not recommended to
 * extend its functionality, as it may be removed altogether in a future release.
 * @class
 * @property {Date} ctime
 * @property {Date} mtime
 * @property {Integer} dev
 * @property {Integer} ino
 * @property {Integer} mode
 * @property {Integer} uid
 * @property {Integer} gid
 * @property {Integer} file_size
 * @property {String} oid
 * @property {Integer} flags
 * @property {Integer} flags_extended
 * @property {String} path
 */
var IndexEntry = function() {};

/**
 * A RevisionWalker provides a clean interface to walk the commit history of a
 * {@link Repository}. A RevisionWalker may be obtained via {@link Repository#createWalker}
 * @class 
 * 
 * @see Repository#createWalker
 */
var RevisionWalker = function() {};

/**
 * Pushes a Commit onto the Revision Walker to begin the walking process.
 * @param {String|Commit} commit The commit to push onto the walker. Can either be an sha1 id string, or a previously loaded Commit.
 * @returns {Boolean} true if successful.
 * @throws If there was an error pushing the Commit.
 */
RevisionWalker.prototype.push = function(commit, callback) {};

/**
 * Hides a specific Commit (and all its ancestors/descendants) from the walker.
 * @param {String|Commit} commit The commit to ignore. Can either be a sha1 id string, or a previously loaded Commit.
 * @param {Function} [callback] If provided, the hide process will be performed in a non-blocking fashion, and the results passed back to the callback.
 * @returns {Boolean} true if successful.
 * @throws If there was an error hiding the Commit.
 */
RevisionWalker.prototype.hide = function(commit, callback) {};

/**
 * Gets the next Commit in the queue from the RevisionWalker.
 * @param {Function} [callback] If provided, the next Commit will be retrieved asynchronously, and the results passed back to callback.
 * @returns {Commit}
 * @throws If an error occurs while getting the next Commit.
 */
RevisionWalker.prototype.next = function(callback) {};

/**
 * Sets a specific sorting method on the RevisionWalker.
 * @param {Integer} sorting The sorting method to use on this walker. The valid values are {@link Gitteh.GIT_SORT_NONE}, {@link Gitteh.GIT_SORT_TOPOLOGICAL}, {@link Gitteh.GIT_SORT_TIME}, and {@link Gitteh.GIT_SORT_REVERSE}.
 * @param {Function} [callback] If provided, walker will be configured with sorting options in a non-blocking manner, and the results of the operation passsed to the callback.
 * @returns {Boolean} true if successful.
 * @throws If an error occurred while setting sorting.
 * @see Gitteh.GIT_SORT_NONE
 * @see Gitteh.GIT_SORT_TOPOLOGICAL
 * @see Gitteh.GIT_SORT_TIME
 * @see Gitteh.GIT_SORT_REVERSE
 */
RevisionWalker.prototype.sort = function(sorting, callback) {};

/**
 * Resets the RevisionWalker to an identity state ready to begin another walk.
 * @param {Function} [callback] If provided, the walker will be reset asynchronously and the results passed to the callback.
 * @returns {Boolean} true if successful.
 * @throws If an error occurred.
 */
RevisionWalker.prototype.reset = function(callback) {};

/**
 * @public
 * @static
 */
Gitteh.GIT_SORT_NONE = undefined;
/**
 * @public
 * @static
 */
Gitteh.GIT_SORT_TOPOLOGICAL = undefined;
/**
 * @public
 * @static
 */
Gitteh.GIT_SORT_TIME = undefined;
/**
 * @public
 * @static
 */
Gitteh.GIT_SORT_REVERSE = undefined;

/**
 * @class
 * @property {String} id <i>Read-only</i> The sha1 hash of this Tag.
 * @property {String} message The message associated with this Tag.
 * @property {String} name The name of this Tag.
 * @property {Signature} tagger The person who created this Tag.
 * @property {String} targetId The object this Tag is pointing at.
 * @property {String} targetType The type of object this Tag is pointing at.
 * 
 * @see Repository#getTag
 * @see Repository#createTag
 */
var Tag = function() {};

/**
 * Saves the data contained in this Tag and updates the ID.
 * @param {Function} [callback] If provided, the Tag will be saved asynchronously and the results provided to the callback.
 * @returns {Boolean} true if successful.
 * @throws If an error occurs during save process.
 */
Tag.prototype.save = function(callback) {};

/**
 * A Signature is used to store metadata about a person. It is used in {@link Commit}s
 * and {@link Tag}s. Note that there is no actual constructor for a Signature,
 * it's just a regular JS object. This documentation just serves as a guide to
 * what properties should be in a Signature. 
 * @class
 * @property {String} name 
 * @property {String} email
 * @property {Date} time
 * @property {Integer} timeOffset In seconds. e.g GMT+10 is stored as 600.
 */
var Signature = function() {};

/**
 * Errors thrown by Gitteh will generally include additional data if it's a Git
 * related issue. Note that there is no actual Error type called GitError, this 
 * is just for illustrative purposes only.
 * @class
 * @property {Integer} gitError The error number. The possible errors are detailed in {@link Gitteh.error}.
 * @property {String} gitErrorStr A short message explaining the error.
 */
var GitError = function() {};