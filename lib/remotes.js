var url = require("url");
var http = require("http");
var https = require("https");
var net = require("net");
var Put = require("put");
var fs = require("fs");

String.prototype.leftPad = function (l, c) { return new Array(l - this.length + 1).join(c || '0') + this; }

var Remote = module.exports = function() {
	
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

var HTTPRemoteEndpoint = function(repoUrl) {
	var get;

	// Which http client are we using?
	if(repoUrl.protocol == "https:") {
		get = https.get;
	}
	else {
		get = http.get;
	}

	this.clone = function() {
		// Let's get this party started. Query the HEAD ref.
		
	};

};

/**
 * Clones a remote Repository on the local filesystem.
 * @param {String} repoUrl The URL to the repo being cloned.
 * @param {String} path The local filesystem path where repo should be cloned to.
 * @param {Function}
 */
Remote.clone = function(repoUrl, repoPath, callback) {
	repoUrl = url.parse(repoUrl);
	
	console.log(repoUrl);process.exit(-1);
	
	if(repoUrl.protocol == "git:") {
		var remote = new GitRemoteEndpoint(repoUrl);
		remote.clone();
	}
	
	if(repoUrl.protocol == "http:" || repoUrl.protocol == "https:") {
		var remote = new HTTPRemoteEndpoint(repoUrl);
		remote.clone();
	}
};