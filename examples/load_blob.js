gitteh = require "../lib/gitteh"
path = require "path"

gitteh.openRepository path.join(__dirname, ".."), (err, repo) ->
	repo.blob "aa4306ebb3e97b8ec76136feab1bb5fcd096b28a", (err, blob) ->
		console.log blob
