gitteh = require "../lib/gitteh"
path = require "path"

gitteh.openRepository path.join(__dirname, ".."), (err, repo) ->
	repo.commit "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, commit) ->
		commit.tree (err, tree) ->
			console.log tree