path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"

selfRepo = path.join __dirname, "../"
selfRepoGitPath = path.join selfRepo, ".git/"

describe "Repository", ->
	describe "Repository", ->
		describe "Using the project repo...", ->
			repo = null
			it "opens correctly", (done) ->
				gitteh.openRepository selfRepo, (err, _repo) ->
					repo = _repo
					repo.should.be.an.instanceof gitteh.Repository
					done()
			describe "#bare", ->
				it "should be false for this repo", ->
					repo.bare.should.be.false
				it "should be immutable", ->
					repo.bare = 123
					repo.bare.should.be.false
					delete repo.path
					repo.bare.should.be.false
			describe "#path", ->
				it "should point to #{selfRepoGitPath}", ->
					repo.path.should.equal selfRepoGitPath
				it "should be immutable", ->
					repo.path = "foo"
					repo.path.should.equal selfRepoGitPath
					delete repo.path
					repo.path.should.equal selfRepoGitPath
			describe "#workDir", ->
				it "should point to #{selfRepo}", ->
					repo.workDir.should.equal selfRepo
				it "should be immutable", ->
					repo.path = "foo"
					repo.workDir.should.equal selfRepo
					delete repo.path
					repo.workDir.should.equal selfRepo
			describe "#exists()", ->
				it "should return true for first commit in repo :)", (done) ->
					repo.exists "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, exists) ->
						exists.should.be.true
						done()
