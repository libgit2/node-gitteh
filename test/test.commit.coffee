path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
utils = require "./utils"

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
			it "is immutable", -> utils.checkImmutable commit, "id"
		describe "#treeId", ->
			it "is correct", ->
				commit.treeId.should.be.equal treeId
			it "is immutable", -> utils.checkImmutable commit, "treeId"
		describe "#message", ->
			it "is correct", ->
				commit.message.should.be.equal 
			it "is immutable", -> utils.checkImmutable commit, "message"
		describe "#parents", ->
			it "is correct", ->
				commit.parents.should.have.length 1
				commit.parents.should.include parentId
		describe "#author", ->
			it "is valid", ->
				author = commit.author
				author.should.be.a "object"
				should.exist author.name
				should.exist author.email
				should.exist author.time
				should.exist author.offset
			it "is immutable", -> utils.checkImmutable commit, "author"
			describe "#name", ->
				it "is valid", ->
					commit.author.name.should.be.equal "Sam"
				it "is immutable", -> utils.checkImmutable commit.author, "name"
			describe "#email", ->
				it "is valid", ->
					commit.author.email.should.be.equal "sam.c.day@gmail.com"
				it "is immutable", -> utils.checkImmutable commit.author, "email"
			describe "#time", ->
				it "is valid", ->
					commit.author.time.toUTCString().should.be.equal "Sat, 26 Feb 2011 02:39:48 GMT"
				it "is immutable", -> utils.checkImmutable commit.author, "time"
			describe "#offset", ->
				it "is valid", ->
					commit.author.offset.should.equal 600
				it "is immutable", -> utils.checkImmutable commit.author, "offset"
		describe "#committer", ->
			it "is valid", ->
				committer = commit.committer
				committer.should.be.a "object"
				should.exist committer.name
				should.exist committer.email
				should.exist committer.time
				should.exist committer.offset
			it "is immutable", -> utils.checkImmutable commit, "committer"
			describe "#name", ->
				it "is valid", ->
					commit.committer.name.should.be.equal "Sam"
				it "is immutable", -> utils.checkImmutable commit.committer, "name"
			describe "#email", ->
				it "is valid", ->
					commit.committer.email.should.be.equal "sam.c.day@gmail.com"
				it "is immutable", -> utils.checkImmutable commit.committer, "email"
			describe "#time", ->
				it "is valid", ->
					commit.committer.time.toUTCString().should.be.equal "Sat, 26 Feb 2011 02:39:48 GMT"
				it "is immutable", -> utils.checkImmutable commit.committer, "time"
			describe "#offset", ->
				it "is valid", ->
					commit.committer.offset.should.equal 600
				it "is immutable", -> utils.checkImmutable commit.committer, "offset"
