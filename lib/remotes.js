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
var zlib = require("zlib");
var path = require("path");
var events = require("events");
var util = require("util");
var temp = require("temp");

String.prototype.leftPad = function (l, c) { return new Array(l - this.length + 1).join(c || '0') + this; };

String.prototype.endsWith = function(suffix) {
    return this.indexOf(suffix, this.length - suffix.length) !== -1;
};

String.prototype.startsWith = function(prefix) {
    return this.indexOf(prefix) === 0;
};

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

var checkoutHEAD = function(repo, callback) {

	async.waterfall([
		function(callback) { repo.getReference("HEAD", callback); },
		function(ref, callback) { ref.resolve(callback); },
		function(ref, callback) { repo.getCommit(ref.target, callback); },
		function(headCommit, callback) { repo.getTree(headCommit.tree, callback); },
		function(tree, callback) {
			var extractTree = function(tree, exportPath) {
				return function(callback) {
					if(!exportPath) exportPath = "/";
					
					var next = function() {
						var fns = [];
						tree.entries.forEach(function(entry) {
							if(entry.attributes == 16384) {
								repo.getTree(entry.id, function(err, tree) {
									fns.push(extractTree(tree, path.join(exportPath, entry.name)));	
								});
							}
							else {
								fns.push(function(callback) {
									async.waterfall([
										function(callback) { repo.getBlob(entry.id, callback); },
										function(blob, callback) {
											fs.writeFile(path.join(repo.path, exportPath, entry.name), blob.data, "binary", callback);
										}
									], callback);
								});
							}
						});
	
						async.parallel(fns, callback);
					};
	
					path.exists(path.join(repo.path, exportPath), function(exists) {
						if(!exists) {
							fs.mkdir(path.join(repo.path, exportPath), 777, function(err) {
								if(err) return callback(err);
								next();
							});
						}
						else {
							next();
						}
					});
				};
			};

			extractTree(tree)(callback);
		}
	], callback);
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
	// We need to convert line to a Buffer because null-terminated strings in JS
	// act fucking weird (concat operations with a \0 on the end will be automagically
	// stripped off.
	var data;
	if(!(line instanceof Buffer)) {
		data = new Buffer(line, "binary");
	}

	var length = (data.length + 4).toString(16).leftPad(4);

	connection.write(length);
	connection.write(data);
};

var readPktLines = function(connection, callback) {
	var listener = function(data) {
		var index = 0;
		var lines = [];

		while(index < data.length) {
			var length = parseInt(data.toString("utf8", index, index + 4), 16);
			index += 4;

			if(length == 0) {
				// Flush pkt.
				lines.push(null);
			}
			else {
				lines.push(data.slice(index, index + length - 4));
				index += length - 4;
			}
		}
		
		callback(undefined, lines);
	};

	connection.once("data", listener);
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
			var lineLength = 
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

var PktLineHandler = function(connection) {
	var lineQueue = [];

	var lineWaiter = null;
	
	var addLineToQueue = function(line) {
		if(lineWaiter) {
			var theWaiter = lineWaiter;
			lineWaiter = null;
			theWaiter(line);
		}
		else {
			lineQueue.push(line);
		}
	};

	var length = -1;
	var lengthFragment = "";
	var buf = null;
	var bufIndex = 0;
	var handler = function(data) {
		var i = 0;
		while(i < data.length) {
			if(length < 0) {
				if(lengthFragment) {
					var remaining = Math.min(4 - lengthFragment.length, data.length);
					lengthFragment += data.toString("utf8", i, i + remaining);
					i += remaining;
					
					if(lengthFragment.length == 4) {
						length = parseInt(lengthFragment, 16) - 4;
						lengthFragment = "";
					}
				}
				else {
					if((i + 4) > data.length) {
						// Bahhhhh this sucks. There's a slight chance the 4 bytes
						// of length get split between two packets.
						lengthFragment = data.toString("utf8", i, data.length);
						i = data.length;
						break;
					}
	
					length = parseInt(data.toString("utf8", i, i + 4), 16);
					i += 4;
					
					// Is this a flush packet?
					if(length == 0) {
						addLineToQueue(null);
						length = -1;
						continue;
					}
	
					length -= 4;
				}
			}
			else {
				// Are we in the middle of building a line from the last packet?
				if(!buf) {
					// Do we have the whole line now, in this one packet? If so we can
					// slice() the data.
					// If not, we'll have to allocate a new Buffer for it and copy.
					if(length > (data.length - i)) {
						buf = new Buffer(length);
						var amt = data.copy(buf, 0, i, data.length);
						i += amt;
						bufIndex = amt;
					}
					else {
						addLineToQueue(data.slice(i, i + length));
						i += length;
						length = -1;
					}
				}	
				else {
					var amt = data.copy(buf, bufIndex, i, Math.min(length - bufIndex, data.length - i));
					i += amt;
					bufIndex += amt;
					
					if(bufIndex == length) {
						addLineToQueue(buf);
						buf = null;
						bufIndex = 0;
						length = -1;
					}
				}
			}
		}
	};
	
	this.getLine = function(callback) {
		if(lineQueue.length) {
			var line = lineQueue.shift();
			process.nextTick(function() {
				callback(line);
			});
		}
		else {
			// Should only be one consumer using this thing at a time.
			lineWaiter = callback;
		}
	};

	connection.on("data", handler);
};

// Handles receiving data in side-band/side-band-64k mode.
var SidebandReceiver = function(pktLineHandler) {
	var that = this;

	var handler = function(line) {
		// First byte denotes the sideband this data is for.
		// 1 - packfile data, 2 - stderr debug, 3 - ohshit message
		if(line === null) {
			return;
		}

		var mode = line[0];
		if(mode < 1 || mode > 3) {
			// TODO: error handling.
			throw Error("Invalid sideband received: " + mode);
		}

		if(mode == 1) {
			that.emit("data", line.slice(1));
			
			// Give us some more of that sweet data lovin'.
			pktLineHandler.getLine(handler);
		}
		else {
			if(mode == 2) {
				that.emit("info", line.slice(1).toString());
				pktLineHandler.getLine(handler);
			}
			else {
				console.log(line);
				that.emit("error", line.slice(1).toString());
			}
		}
	};
	
	pktLineHandler.getLine(handler);
};
util.inherits(SidebandReceiver, events.EventEmitter);

var getAdvertisedRefs = function(pktLineHandler, callback) {
	var refs = [];
	var capabilities = [];
	var firstLine = true;
	var handler = function(line) {
		// TODO: handle no-refs situation.

		if(line == null) {
			// Flush packet. We now have a complete list.
			callback(refs, capabilities);
			return;
		}

		var ref = line.toString();
		if(firstLine) {
			var parts = ref.split("\0");
			ref = parts[0];

			capabilities = parts[1].trim().split(" ");
			firstLine = false;
		}
		
		ref = ref.split(" ");
		refs.push({
			id: ref[0].trim(),
			name: ref[1].trim()
		});

		pktLineHandler.getLine(handler);
	};
	
	pktLineHandler.getLine(handler);
};

var GitRemoteEndpoint = function(repo, repoUrl) {
	var that = this;

	this.fetch = function(callback) {		
		var connection = net.createConnection(9418, repoUrl.host);
		
		connection.on("close", function() {
			console.log("connection closed.", arguments);
		});
		
		connection.on("error", function() {
			console.log("Connection error.");
		});

		connection.on("connect", function() {
			// Repo path in url has a trailing slash which we don't want
			var pathName = repoUrl.pathname.substring(0, repoUrl.pathname.length-1);
			// Build the request string.
			var request = "git-upload-pack " + pathName + "\0" + "host=" +
					repoUrl.host;
			// Apparently appending "\0" to the end of a string when initializing
			// it helpfully doesn't append it.
			request += "\0";

			// Send the request to server.
			sendPktLine(request, connection);
			
			var pktLineHandler = new PktLineHandler(connection);
			
			getAdvertisedRefs(pktLineHandler, function(refs, capabilities) {
				// We only support sideband and sideband 64k mode.
				if(capabilities.indexOf("side-band") == -1 && capabilities.indexOf("side-band-64k") == -1) {
					return callback(new Error("Server does not support side-band mode."));
				}

				// TODO: process all refs and setup functionality to find a common
				// ancestor between client/server using multi_ack. This will be 
				// necessary to do a fetch rather than a straight clone.

				var id = refs[0].id;
				console.log("Requesting commit '" +id + "'");

				sendPktLine("want " + id + " side-band-64k\n", connection);
				connection.write("0000");
				sendPktLine("done", connection);
	
				pktLineHandler.getLine(function(line) {
					var packfileTemp = temp.openSync();
					var packfileStream = fs.createWriteStream(packfileTemp.path, {
						fd: packfileTemp.fd
					});
					var receiver = new SidebandReceiver(pktLineHandler);
					receiver.on("data", function(data) {
						//console.log("OUT> ", " <BINARY>");
					});
					receiver.on("info", function(info) {
						//console.log("ERR>", info);
						process.stdout.write(info);
					});
					receiver.on("error", function(error) {
						console.log("FAILURE!", error);
					});
				});
			});
		});
	};

	this.clone = function(callback) {
		that.fetch(callback);
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
	
	var activeConnections = [];

	var closeHTTPConnections = function(url) {
		var agent = http.getAgent(url.hostname, url.port);
		agent.sockets.forEach(function(socket) {
			socket.destroy();
		});
	};
	
	var doGet = function(resource, baseUrl) {
		return function(callback) {
			var responseData = new BufferList();
	
			var theUrl = url.parse(url.resolve(baseUrl, resource));
			var con = get({
				host: theUrl.hostname,
				path: theUrl.pathname,
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
			});
			con.on("error", function(err) {
				callback(err);
			});
			
			activeConnections.push(con);
		};
	};

	var packList = [];
	var alternates = [];
	
	var populatePackList = function(rawPackList, baseUrl) {
		packNames = rawPackList.split("\n");
		packNames.forEach(function(packSpec) {
			packSpec = packSpec.substring(2);
			var packName = packSpec.substring(0, packSpec.length - 5);
			if(!packName) return;
			
			packList.push({
				name: packName,
				url: baseUrl
			});
		});
	};

	var downloadPackIndexes = function() {
		var tmpDir = path.join(repo.path, ".git", "gittehtmp");
		if(!path.existsSync(tmpDir)) {
			fs.mkdirSync(tmpDir, "777");
			fs.mkdirSync(path.join(tmpDir, "pack"), "777");
		}
		
		// TODO: do this async. Just don't wanna nest my code any more right now
		// while I develop this sucker.
		var odb = gittehBindings.openODB(tmpDir);

		// Any sha1 resolutions that are waiting for ALL indexes to load will
		// be registered here.
		var queue = [];
		var packIndexes = {};
		var fns = [];
		var allDone = false;
		var packs = {};

		var downloadPack = function(packName) {
			if(packs[packName].downloading) return;

			packs[packName].downloading = true;
			
			doGet("pack/" + packName + ".pack", packs[packName].base)(function(err, result) {
				if(err) {
					// TODO: handle errors.
					throw err;
				}

				// Put this pack, and the index, in the objects/pack directory.
				async.parallel([
					function(callback) {
						fs.writeFile(path.join(tmpDir, "pack", packName + ".pack"), result, "binary", callback);
					},
					
					function(callback) {
						fs.writeFile(path.join(tmpDir, "pack", packName + ".idx"), packIndexes[packName].raw, "binary", callback);
					}
				], function(err, result) {
					if(err) {
						// TODO: more eror handling.
						throw err;
					}

					// Reload odb. For now that involvesd recreating it. Ugh.
					odb = gittehBindings.openODB(path.join(tmpDir));
					packs[packName].downloaded = true;

					packs[packName].queue.forEach(function(queueItem) {
						// TODO: make this async.
						try {
							var object = odb.get(queueItem.sha);
							queueItem.fn(null, object.data);
						}
						catch(err) {
							return queueItem.fn(err);
						}
					});
				});
			});
		};

		// The rest of the traversal routines will reference this.
		var packInterface = {
			// Does a brief scan of the index to see if the sha1 is in here.
			checkSha: function(sha1) {
				Object.keys(packIndexes).forEach(function(packName) {
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
				var found = false;
				Object.keys(packIndexes).forEach(function(packName) {
					if(found) return;

					// First step. Is it in our ODB already?
					try {
						var obj = odb.get(sha1);
					}
					catch(e) {
						obj = null;
					}

					if(obj) {
						callback(undefined, obj.data);
					}

					if(packIndexes[packName].sha1s.indexOf(sha1) > -1) {
						found = true;
						// Well, we've found the object. It's in this pack.
						// But, pack isn't downloaded yet (if it was, odb.getObject
						// woulda worked). We'll queue up this object to have its
						// contents delivered once pack has finished downloading.
						packs[packName].queue.push({
							sha: sha1,
							fn: callback
						});
						downloadPack(packName);

						return;
					}
				});

				if(!found) {
					if(!allDone) {
						// Not all pack indexes are loaded yet, queue this resolution up.
						queue.push({
							sha: sha1,
							fn: callback
						});
					}
					else {
						callback(undefined, null);
					}
				}
			}
		};
		
		packList.forEach(function(pack) {
			var packName = pack.name;

			packs[packName] = {
				downloaded: false,
				downloading: false,
				queue: [],
				base: pack.url
			};

			fns.push(function(callback) {
				doGet("pack/" + packName + ".idx", pack.url)(function(err, result) {
					// If there's an error, we won't do anything here. Instead
					// when an object needs to resolve, and it can't because it's
					// in this failed index, we'll know then.
					if(err && !result) return;
					
					packIndexes[packName] = new PackfileIndex();
					packIndexes[packName].parse(new Buffer(result, "binary"));

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
		var packs = downloadPackIndexes();

		var objects = {};

		var processCommit = function(sha1, callback) {
			var commit = repo.getCommit(sha1);

			var fns = [];
			commit.parents.forEach(function(parentId) {
				fns.push(getObject(parentId, "commit"));
			});
			fns.push(getObject(commit.tree, "tree"));
			
			async.parallel(fns, function(err) {
				if(err) return callback(err);
				
				callback(null, !commit.parents.length);
			});
		};
		
		var processTree = function(sha1, callback) {
			var tree = repo.getTree(sha1);

			var fns = [];
			
			tree.entries.forEach(function(entry) {
				if(entry.attributes == 16384) {
					fns.push(getObject(entry.id, "tree"));
				}
				else {
					fns.push(getObject(entry.id, "blob"));
				}
			});
			
			async.parallel(fns, callback);
		};
		
		var processTag = function(sha1, callback) {
			var tag = repo.getTag(sha1);
			getObject(tag.targetId, tag.targetType)(callback);
		};

		var processObject = function(sha1, type, data, callback) {
			var obj = repo.odb.create({
				type: type,
				data: data
			});
			
			if(obj.id != sha1) {
				return callback(new Error("Created " + type + " has mismatching sha1! Expected " + sha1 + ". got " + obj.id));
			}
			
			// Right. We've succesfully got a valid object up in this bitch.
			// Let's traverse any trees/commits/tags/whatever!
			if(obj.type == "commit") {
				processCommit(sha1, /*function(err, isRootCommit) {
					if(err) return callback(err);
					
					console.log(isRootCommit + " " + sha1);
					if(isRootCommit) {
						callback(null, true);
					}
				}*/callback);
			}
			else if(obj.type == "tree") {
				processTree(sha1, callback);
			}
			else if(obj.type == "tag") {
				processTag(sha1, callback);
			}
			else if(obj.type == "blob") {
				callback(null, true); 
			}
		};

		var getLoose = function(sha1, type) {
			return function(callback) {
				var path = "objects/" + loosePathFormat(sha1);
		
				doGet(path, repoUrl)(function(err, data) {
					if(err || !data) {
						return callback(null);
					}
		
					// inflate object, process it.
					var inflated = new Buffer(zlib.inflate(new Buffer(data, "binary")));
					
					// strip off header.
					var i = 0;
					while(i < inflated.length) {
						if(inflated[i] == 0) break;
						i++;
					}
					
					inflated = inflated.slice(i + 1);
					callback(null, inflated)
				});
			};
		};

		var getObject = function(sha1, type) {
			console.log("Getting " + type + " " + sha1);

			return function(callback) {
				// First step, make sure we haven't already resolved this object.
				if(objects[sha1]) return callback(null, true);

				// Otherwise, mark us as on the job!
				objects[sha1] = true;
				
				// First step. Is it in one of the indexes?
				if(packs.checkSha(sha1)) {
					packs.loadSha(sha1, function(err, data) {
						processObject(sha1, type, data, callback);
					});
				}

				// Either packfile indexes haven't loaded yet, or it's definitely
				// not in there. Let's check loose storage.
				getLoose(sha1, type)(function(err, data) {
					if(err) return callback(err);
					
					if(data) {
						processObject(sha1, type, data, callback);
					}
					else {
						// Definitely not in loose. HAS to be in packfile.
						packs.loadSha(sha1, function(err, data) {
							if(err) return callback(err);
							if(!data) return callback(new Error("Couldn't find data for " + sha1));
							processObject(sha1, type, data, callback);
						});
					}
				});
			};
		};
		
		var refs = results.refs.split("\n")
			.filter(function(refSpec) { return refSpec.length; })
			.map(function(refSpec) { return refSpec.split("\t"); });

		var refNames = refs.map(function(refSpec) { return refSpec[1]; });
		
		// Now let's start loading all objects. We kick multiple requests into
		// gear for each ref.
		var fns = [];
		refs.forEach(function(ref) {
			// Refs will be pointing at commits. Unless they're pointing at a tag.
			// They're pointing at a tag if they have a peeled ref included in the
			// list. Otherwise they're cheapo tags that aren't actually an annotation
			// tag object.
			
			var refName = ref[1];
			var refId = ref[0];
			
			var type = "commit";
			if(refName.startsWith("refs/tags/")) {
				if(refName.endsWith("^{}")) {
					// Type is still a commit.
				}
				else if(refNames.indexOf(refName + "^{}") > -1) {
					type = "tag";
				}
			}
			
			fns.push(getObject(refId, type));
		});

		async.parallel(fns, function(err) {
			if(err) return callback(err);
			
			// Make sure all references are setup.
			refs.forEach(function(ref) {
				repo.createOidReference(ref[1], ref[0]);
			});

			//console.log(results.head.replace("ref: ", "").replace("\n", ""));
			//repo.createSymbolicReference("HEAD", results.head.replace("ref: ", "").replace("\n", ""));
			fs.writeFileSync(path.join(repo.path, ".git", "HEAD"), results.head);

			// Last step, checkout a working copy of the HEAD commit.
			checkoutHEAD(repo, callback);

			/*// Close the active connections.
			alternates.forEach(function(alternateUrl) {
				closeHTTPConnections(alternateUrl);
			});
			
			closeHTTPConnections(repoUrl);*/
			
			//callback(undefined, null);
		});
	};

	this.clone = function(callback) {
		// Let's get this party started. Query HEAD, refs, and index info.
		async.parallel({
			head: doGet("HEAD", repoUrl),
			refs: doGet("info/refs", repoUrl),
			pack: doGet("objects/info/packs", repoUrl),
			alts: doGet("objects/info/http-alternates", repoUrl)
		}, function(err, results) {
			if(err) return callback(err);

			// Add the pack files from primary source.
			populatePackList(results.pack, url.parse(url.resolve(repoUrl, "objects/")));
			
			// Let's process alternates and load the packfiles for it too, before
			// we proceed any further.
			if(results.alts) {
				var altLinks = results.alts.split("\n");
				var fns = [];
				altLinks.forEach(function(altLink) {
					if(!altLink) return;
	
					altLink = url.parse(url.resolve(repoUrl, altLink + "/"));
					alternates.push(altLink);

					fns.push(function(callback) {
						doGet("info/packs", altLink)(function(err, data) {
							// Ignore errors. We may not need this pack anyway.
							if(err || !data) return;
	
							populatePackList(data, altLink);
							callback(undefined);
						});
					});
				});

				async.parallel(fns, function(err) {
					fetch(results, callback);
				});
			}
			else {
				fetch(results, callback); 
			}
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
			repoUrl = url.parse(repoUrl + "/");
			
			if(repoUrl.protocol == "git:") {
				var remote = new GitRemoteEndpoint(repo, repoUrl);
				remote.clone(callback);
			}
			
			if(repoUrl.protocol == "http:" || repoUrl.protocol == "https:") {
				var remote = new HTTPRemoteEndpoint(repo, repoUrl);
				remote.clone(callback);
			}
		}
	], callback);
};