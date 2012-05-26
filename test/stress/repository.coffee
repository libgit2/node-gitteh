# Open 1000 copies of the same repository location in one async batch, ensure
# each one behaves normally.

gitteh = require "../../lib/gitteh"
{projectRepo} = require "../fixtures"
assert = require "assert"
async = require "async"

openCommitAndTree = (i, cb) ->
	gitteh.openRepository projectRepo.path, (err, repo) ->
		assert repo
		async.parallel
			commit: (cb) -> repo.commit projectRepo.secondCommit.id, cb
			tree: (cb) -> repo.tree projectRepo.secondCommit.tree, cb
		, (err, results) ->
			return cb err if err?
			assert results.commit.id is projectRepo.secondCommit.id
			assert results.tree.id is projectRepo.secondCommit.tree
			cb()

openCommitAndTreeSeparately = (i, cb) ->
	async.parallel
		commit: (cb) ->
			gitteh.openRepository projectRepo.path, (err, repo) ->
				if not repo
					console.log err
				assert repo
				repo.commit projectRepo.secondCommit.id, cb
		###
		tree: (cb) ->
			gitteh.openRepository projectRepo.path, (err, repo) ->
				assert repo
				repo.tree projectRepo.secondCommit.tree, cb
		###
	, cb

module.exports = (cb) ->
	async.series
		openCommitAndTree: (cb) ->
			console.time "openCommitAndTree"
			async.forEach [1..1000], openCommitAndTree, () ->
				console.timeEnd "openCommitAndTree"
				cb()
		openCommitAndTreeSeparately: (cb) ->
			console.time "openCommitAndTreeSeparately"
			async.forEach [1..10000], openCommitAndTreeSeparately, () ->
				console.timeEnd "openCommitAndTreeSeparately"
				cb()
