path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"

selfRepo = path.join __dirname, "../"
selfRepoGitPath = path.join selfRepo, ".git/"

commitId = "8a916d5fbce49f5780668a1ee780e0ef2e89360f"
treeId = "aa41780f2129bf03cce1a3eeadc78db47f83d9ad"
parentId = "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14"
commitMsg = "Stuff."

describe "Commit", ->
	repo = null
	commit = null

	describe "Using the project repo...", ->
		it "can find second commit (8a916d5fbce49f5780668a1ee780e0ef2e89360f)", (done) ->
			gitteh.openRepository selfRepo, (err, _repo) ->
				repo = _repo
				repo.commit commitId, (err, _commit) ->
					commit = _commit
					should.not.exist err
					commit.should.be.an.instanceof gitteh.Commit
					done()
	describe "The first project commit...", ->
		describe "#id", ->
			it "is correct", ->
				commit.id.should.be.equal commitId
			it "is immutable", ->
				commit.id = "foo"
				commit.id.should.be.equal commitId
				delete commit.id
				commit.id.should.be.equal commitId
		describe "#treeId", ->
			it "is correct", ->
				commit.treeId.should.be.equal treeId
			it "is immutable", ->
				commit.treeId = "foo"
				commit.treeId.should.be.equal treeId
				delete commit.treeId
				commit.treeId.should.be.equal treeId
		describe "#message", ->
			it "is correct", ->
				commit.message.should.be.equal 
			it "is immutable", ->
				commit.message = "foo"
				commit.message.should.be.equal commitMsg
				delete commit.message
				commit.message.should.be.equal commitMsg
		describe "#parents", ->
			it "is correct", ->
				commit.parents.should.have.length 1
				commit.parents.should.include parentId
