path = require "path"
should = require "should"
temp = require "temp"
gitteh = require "../lib/gitteh"

selfRepo = path.join __dirname, ".."

describe "Gitteh", ->
	describe "#openRepository", ->
		describe "on project repo", ->
			it "should open correctly", (done) ->
				gitteh.openRepository selfRepo, done
			it "should not be bare", (done) ->
				gitteh.openRepository selfRepo, (err, repo) ->
					repo.bare.should.be.false
					done()
			it "should have immutable bare property", (done) ->
				gitteh.openRepository selfRepo, (err, repo) ->
					repo.bare = 123
					repo.bare.should.be.false
					done()
			it "should have correct path to git dir", (done) ->
				gitteh.openRepository selfRepo, (err, repo) ->
					repo.path.should.equal path.join selfRepo, ".git/"
					done()
			it "should have an immutable git path", (done) ->
				gitteh.openRepository selfRepo, (err, repo) ->
					repo.path = "foo"
					repo.path.should.equal path.join selfRepo, ".git/"
					done()
		describe "on an invalid path", ->
			it "should fail with an Exception", (done) ->
				gitteh.openRepository "/i/shouldnt/exist", (err) ->
					err.should.be.an.instanceof Error
					done()
