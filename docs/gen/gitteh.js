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
Gitteh.initRepository = function(path, bare, callback) { };