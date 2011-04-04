var gittehBindings = require("../build/default/gitteh");
var url = require("url");
var http = require("http");
var https = require("https");
var net = require("net");
var fs = require("fs");
var BufferList = require("bufferlist").BufferList;
var async = require("async");
var bufferUtils = require("./buffer_utils.js");
var path = require("path");

String.prototype.leftPad = function (l, c) { return new Array(l - this.length + 1).join(c || '0') + this; };


// FUTURE SAM.
// Tomorrow you need to modify the implementation a bit, and download packfiles
// to a temporary folder. You want to do this so as to future proof this code
// and allow git fetch, not just git clone.

var getSha1Hex = function(data, index) {
	var hex = "";
	for(var i = 0; i < 20; i++) {
		hex += data[index+i].toString(16).leftPad(2);
	}
	
	return hex;
};

var PackfileIndex = function() {
	this.sha1s = [];
	this.raw = null;

	var version = 1;

	this.parse = function(data) {
		this.raw = data;

		index = 0;
		if((bufferUtils.getInt32(data, 0) == 0xff744f63) && (bufferUtils.getInt32(data, 4) == 2)) {
			version = 2;
			console.log("version 2.");
			
			index = 8;
		}
		
		// For now we skip fan out, and jump to the good bit, idx 255, which has
		// the size in it.
		index += 4*255;

		var size = bufferUtils.getInt32(data, index);
		index += 4;
		
		// Now we can get all the sha1's.
		for(var i = 0; i < size; i++) {
			if(version == 1) {
				index += 4;
			}

			this.sha1s.push(getSha1Hex(data, index)); index += 20;
		}
	};
};

var Remote = module.exports = function() {
	
};

var loosePathFormat = function(sha1) {
	return sha1.substring(0, 2) + "/" + sha1.substring(2);
};

var sendPktLine = function(line, connection) {
	line = ("" + (line.length + 5).toString(16)).leftPad(4) + line;
	console.log(line);
	connection.write(line + "\0");
};

var readResponse = function(connection, callback) {
	var lines = [];

	var listener = function(data) {
		// Process response.
		done = false;
		var firstLine;
		var index = 0;
		var serverCapabilities = [];
		while(!done && (index < data.length)) {
			var lineLength = parseInt(data.toString("utf8", index, index+4), 16);
			console.log(lineLength);
			index += 4;
			
			if(lineLength == 0) {
				done = true;
			}
			else {
				var line = data.toString("utf8", index, index + (lineLength - 5));
				index += lineLength - 4;
	
				lines.push(line);
			}
		}
		
		// If we received the flush pkt line, we're done.
		if(done) {
			connection.removeListener("data", listener);
			callback(undefined, lines);
		}
	};
	
	connection.on("data", listener);
};

var GitRemoteEndpoint = function(repoUrl) {
	var that = this;

	var connection = net.createConnection(9418, repoUrl.host);
	connection.on("connect", function() {
		that.request();
	});

	this.request = function() {		
		// Build the request string.
		var request = "git-upload-pack " + repoUrl.pathname + "\0" + "host=" +
				repoUrl.host;

		sendPktLine(request, connection);
		readResponse(connection, function(err, lines) {
			var head = lines[0].split("\0");
			
			var ref = head[0].split(" ");
			console.log("Requesting commit '" +ref[0] + "'");
			
			sendPktLine("want " + ref[0] + "\0ofs_delta\n", connection);
			connection.write("0000");
			sendPktLine("done", connection);
			
			var out = fs.createWriteStream("packkkkk");
			connection.on("data", function(data) {
				out.write(data);
				out.flush();
			});
		});
		
		setTimeout(function() {
		}, 3000);
	};

	this.clone = function() {
		
	};
};

var HTTPRemoteEndpoint = function(repo, repoUrl) {
	var get;
	
	// Which http client are we using?
	if(repoUrl.protocol == "https:") {
		get = https.get;
	}
	else {
		get = http.get;
	}
	
	var doGet = function(resource) {
		return function(callback) {
			var responseData = new BufferList();
	
			get({
				host: repoUrl.hostname,
				path: repoUrl.pathname + "/" + resource,
				headers: {
					"Connection": "keep-alive"
				}
			}, function(res) {
				if(res.statusCode == 404) {
					return callback(undefined, null);
				}
				else if(res.statusCode != 200) {
					return callback(new Error(resource + " HTTP " + res.statusCode));
				}

				res.on("data", function(data) {
					responseData.push(data);
				});
				res.on("end", function() {
					callback(undefined, responseData.toString());
				});
			}).on("error", function(err) {
				callback(err);
			});
		};
	};

	var downloadPackIndexes = function(packNames) {
		packNames = packNames.split("\n");

		// TODO: do this async. Just don't wanna nest my code any more right now
		// while I develop this sucker.
		var odb = gittehBindings.openODB(path.join(repo.path, ".git", "objects"));

		// Any sha1 resolutions that are waiting for ALL indexes to load will
		// be registered here.
		var queue = [];
		var packIndexes = {};
		var fns = [];
		var allDone = false;
		var packs = {};

		var downloadPack = function(packName) {
			doGet("objects/pack/" + packName + ".pack")(function(err, result) {
				if(err) {
					// TODO: handle errors.
					throw err;
				}

				// Put this pack, and the index, in the objects/pack directory.
				async.parallel([
					function(callback) {
						fs.writeFile(path.join(repo.path, ".git", "objects", "pack", packName + ".pack"), result, "binary", callback);
					},
					
					function(callback) {
						fs.writeFile(path.join(repo.path, ".git", "objects", "pack", packName + ".pack"), packIndexes[packName].raw, "binary", callback);
					}
				], function(err, result) {
					if(err) {
						// TODO: more error handling.
						throw err;
					}

					// Reload odb. For now that involved recreating it. Ugh.
					odb = gittehBindings.openODB(path.join(repo.path, ".git", "objects"));
					packs[packName].downloaded = true;

					packs[packName].queue.forEach(function(queueItem) {
						
					});
				});
			});
		};

		// The rest of the traversal routines will reference this.
		var packInterface = {
			// Does a brief scan of the index to see if the sha1 is in here.
			checkSha: function(sha1) {
				packIndexes.keys().forEach(function(packName) {
					if(packIndexes[packName].sha1s.indexOf(sha1) > -1) {
						return true;
					}
				});
				
				return false;
			},
			
			// Loads object for given sha1 id. This will not return until object
			// is found an loaded from packfile. If all packfile indexes finish
			// downloading, and it's nowhere to be found, we give up then.
			loadSha: function(sha1, callback) {
				packIndexes.keys().forEach(function(packName) {
					// First step. Is it in our ODB already?
					odb.getObject(sha1, function(err, obj) {
						if(!err) {
							callback(undefined, obj.data);
						}
					});

					if(packIndexes[packName].sha1s.indexOf(sha1) > -1) {
						// Well, we've found the object. It's in this pack.
						// But, pack isn't downloaded yet (if it was, odb.getObject
						// woulda worked. We'll queue up this object to have its
						// contents delivered once pack has finished downloading.
						packs[packName].queue.push({
							sha: sha1,
							fn: callback
						});

						return;
					}
				});

				if(callback) {
					if(!allDone) {
						// Not all pack indexes are loaded yet, queue this resolution up.
						queue.push({
							sha: sha1,
							fn: callback
						});
					}
					else {
						callback(null);
					}
				}
			}
		};
		
		packNames.forEach(function(packSpec) {
			packSpec = packSpec.substring(2);
			var packName = packSpec.substring(0, packSpec.length - 5);
			if(!packName) return;

			packs[packName] = {
				downloaded: false,
				queue: []
			};

			fns.push(function(callback) {
				doGet("objects/pack/" + packName + ".idx")(function(err, result) {
					// If there's an error, we won't do anything here. Instead
					// when an object needs to resolve, and it can't because it's
					// in this failed index, we'll know then.
					if(err && !result) return;
					
					var packIndexes = new PackfileIndex();
					packIndexes.parse(new Buffer(result, "binary"));
					
					callback(undefined, true);
				});
			});
		});

		async.parallel(fns, function(err, results) {
			// Process queue now.
			allDone = true;

			queue.forEach(function(queueItem) {
				packInterface.loadSha(queueItem.sha, queueItem.fn);
			});
		});
		
		return packInterface;
	};

	var fetch = function(results, callback) {
		// We'll start building the repo locally, but first, let's queue up
		// download of all the packfile indexes. Since that can happen async
		// and we're probably gonna need them.
		var packs = downloadPackIndexes(results.pack);

		var objects = {};

		var processObject = function(sha1, data, callback) {
			console.log("processing obj", arguments);
		};

		var getObject = function(sha1) {
			return function(callback) {
				// First step, make sure we haven't already resolved this object.
				if(objects[sha1]) return callback(null, true);

				// Otherwise, mark us as on the job!
				objects[sha1] = true;
				
				// First step. Is it in one of the indexes?
				if(packs.checkSha(sha1)) {
					packs.loadSha(sha1, function() {
						console.log("pack obj:", arguments);
					});
				}

				// Either packfile indexes haven't loaded yet, or it's definitely
				// not in there. Let's check loose storage.
				var path = "objects/" + loosePathFormat(sha1);

				doGet(path)(function(err, data) {
					if(err || !data) {
						packs.loadSha(sha1, function() {
							console.log("pack obj:", arguments);
						});
						
						return;
					}
					
					// inflate object, process it.
					processObject(sha1, data, callback);
				});
			};
		};
		
		var refs = results.refs.split("\n")
			.filter(function(refSpec) { return refSpec.length; })
			.map(function(refSpec) { return refSpec.split("\t"); });

		// Now let's start loading all objects. We kick multiple requests into
		// gear for each ref.
		var fns = [];
		refs.forEach(function(ref) {
			fns.push(getObject(ref[0]));
		});
		
		console.log("starting this shit.");
		async.parallel(fns, function(err, results) {
			console("well?", arguments);
		});
	};

	this.clone = function(callback) {
		// Let's get this party started. Query HEAD, refs, and index info.
		async.parallel({
			head: doGet("HEAD"),
			refs: doGet("info/refs"),
			pack: doGet("objects/info/packs"),
			alts: doGet("objects/info/http-alternates")
		}, function(err, results) {
			if(err) return callback(err);

			fetch(results, callback);
		});
	};
};

/**
 * Clones a remote Repository on the local filesystem.
 * @param {String} repoUrl The URL to the repo being cloned.
 * @param {String} path The local filesystem path where repo should be cloned to.
 * @param {Function}
 */
Remote.clone = function(repoUrl, repoPath, callback) {
	async.waterfall([
		function(callback) {
			gittehBindings.initRepository(repoPath, false, callback);
		},
		
		function(repo, callback) {
			repoUrl = url.parse(repoUrl);

			if(repoUrl.protocol == "git:") {
				var remote = new GitRemoteEndpoint(repo, repoUrl);
				remote.clone();
			}
			
			if(repoUrl.protocol == "http:" || repoUrl.protocol == "https:") {
				var remote = new HTTPRemoteEndpoint(repo, repoUrl);
				remote.clone();
			}
		}
	], function(result) {
		console.log(arguments);
	});
};