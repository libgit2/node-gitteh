path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
fixtures = require "./fixtures"

###
describe "Repository", ->
	describe "Using the project repo...", ->
		repo = null
		it "opens correctly", (done) ->
			gitteh.openRepository fixtures.projectRepo.path, (err, _repo) ->
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
			it "should point to #{fixtures.projectRepo.gitPath}", ->
				repo.path.should.equal fixtures.projectRepo.gitPath
			it "should be immutable", ->
				repo.path = "foo"
				repo.path.should.equal fixtures.projectRepo.gitPath
				delete repo.path
				repo.path.should.equal fixtures.projectRepo.gitPath
		describe "#workDir", ->
			it "should point to #{fixtures.projectRepo.path}", ->
				repo.workDir.should.equal fixtures.projectRepo.path
			it "should be immutable", ->
				repo.path = "foo"
				repo.workDir.should.equal fixtures.projectRepo.path
				delete repo.path
				repo.workDir.should.equal fixtures.projectRepo.path
		describe "#exists()", ->
			it "should return true for first commit in repo :)", (done) ->
				repo.exists "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, exists) ->
					exists.should.be.true
					done()
###