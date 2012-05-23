path = require "path"
wrench = require "wrench"
temp = require "temp"
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
