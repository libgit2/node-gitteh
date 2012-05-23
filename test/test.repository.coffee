path = require "path"
should = require "should"
temp = require "temp"
wrench = require "wrench"
gitteh = require "../lib/gitteh"

selfRepo = path.join __dirname, ".."
selfRepoGitPath = path.join selfRepo, ".git/"

describe "Gitteh", ->
	describe "#openRepository()", ->
		describe "on an invalid path", ->
			it "should fail with an Exception", (done) ->
				gitteh.openRepository "/i/shouldnt/exist", (err) ->
					err.should.be.an.instanceof Error
					done()
		describe "on project repo", ->
			repo = null
			it "should open correctly", (done) ->
				gitteh.openRepository selfRepo, (err, _repo) ->
					repo = _repo
					repo.should.be.an.instanceof gitteh.Repository
					done()
	describe "#initRepository()", ->
		describe "on an invalid path", ->
			it "should fail with an Exception", (done) ->
				gitteh.initRepository "/i/shouldnt/exist", (err) ->
					err.should.be.an.instanceof Error
					done()
		describe "initializing a bare repository", ->
			tempPath = "#{temp.path()}/"
			repo = null
			describe "to temp location #{tempPath}", ->
				after ->
					wrench.rmdirSyncRecursive tempPath, true
				it "should initialize correctly", (done) ->
					gitteh.initRepository tempPath, true, (err, _repo) ->
						repo = _repo
						repo.should.be.an.instanceof gitteh.Repository
						done()
				it "should definitely be bare", ->
					repo.bare.should.be.true
				it "should be in the right place", ->
					repo.path.should.be.equal tempPath

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
			describe "#path", ->
				it "should point to #{selfRepoGitPath}", ->
					repo.path.should.equal selfRepoGitPath
				it "should be immutable", ->
					repo.path = "foo"
					repo.path.should.equal selfRepoGitPath
			describe "#exists()", ->
				it "should return true for first commit in repo :)", (done) ->
					repo.exists "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, exists) ->
						exists.should.be.true
						done()
