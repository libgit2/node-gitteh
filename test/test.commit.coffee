path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"

selfRepo = path.join __dirname, "../"
selfRepoGitPath = path.join selfRepo, ".git/"

describe "Commit", ->
	repo = null
	commit = null

	describe "Using the project repo...", ->
		it "can find first project commit", (done) ->
			gitteh.openRepository selfRepo, (err, _repo) ->
				repo = _repo
				repo.commit "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, _commit) ->
					commit = _commit
					should.not.exist err
					commit.should.be.an.instanceof gitteh.Commit
					done()
	describe "The first project commit...", ->
		it "has correct oid", ->
			commit.id.should.be.equal "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14"
		it "has correct tree id", ->
			commit.treeId.should.be.equal "753b8f0db281c50b692ff0f94cd2a614cfdd41b6"