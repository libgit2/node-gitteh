gitteh = require "../../lib/gitteh"
{projectRepo} = require "../fixtures"


gitteh.openRepository projectRepo.path, (err, repo) ->
	repo.exists projectRepo.secondCommit.id, (err, exists) ->
		repo.commit projectRepo.secondCommit.id, (err, commit) ->
			repo.commit projectRepo.secondCommit.id, (err, commit) ->
				console.log "sigh."

				#repo.blob projectRepo.secondCommit.wscriptBlob, (err, blob) ->
				#	console.log blob