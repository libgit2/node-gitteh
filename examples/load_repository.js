gitteh = require "../lib/gitteh"
path = require "path"

gitteh.openRepository path.join(__dirname, ".."), (err, repo) ->
	console.log repo
