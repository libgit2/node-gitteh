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
				return cb err if err?
				assert repo
				repo.commit projectRepo.secondCommit.id, cb
		###
		tree: (cb) ->
			gitteh.openRepository projectRepo.path, (err, repo) ->
				assert repo
				repo.tree projectRepo.secondCommit.tree, cb
		###
	, cb

sameCommit = (i, cb) ->
	gitteh.openRepository projectRepo.path, (err, repo) ->
		return cb err if err?

		async.forEach [1..1000], (i, cb) ->
			repo.commit projectRepo.secondCommit.id, (err, commit) ->
				return cb err if err?
				assert commit.id is projectRepo.secondCommit.id
				# console.log cb
				cb()
			repo.tree projectRepo.secondCommit.tree, (err, tree) ->
				return cb err if err?
				assert tree.id is projectRepo.secondCommit.tree
				# console.log cb
				cb()
		, () ->
			# gc()
			cb()

module.exports = (cb) ->
	async.series
		openCommitAndTree: (cb) ->
			console.time "openCommitAndTree"
			async.forEach [1..1000], openCommitAndTree, (err) ->
				return cb err if err?
				console.timeEnd "openCommitAndTree"
				cb()
		openCommitAndTreeSeparately: (cb) ->
			console.time "openCommitAndTreeSeparately"
			async.forEach [1..1000], openCommitAndTreeSeparately, (err) ->
				return cb err if err?
				console.timeEnd "openCommitAndTreeSeparately"
				cb()
		sameCommit: (cb) ->
			console.time "sameCommit"
			async.forEach [1..1000], sameCommit, (err) ->
				return cb err if err?
				console.timeEnd "sameCommit"
				cb()
	, cb
