path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
fixtures = require "./fixtures"

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
		describe "#workingDirectory", ->
			it "should point to #{fixtures.projectRepo.path}", ->
				repo.workingDirectory.should.equal fixtures.projectRepo.path
			it "should be immutable", ->
				repo.path = "foo"
				repo.workingDirectory.should.equal fixtures.projectRepo.path
				delete repo.path
				repo.workingDirectory.should.equal fixtures.projectRepo.path
		describe "#exists()", ->
			it "should return true for first commit in repo :)", (done) ->
				repo.exists "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, exists) ->
					exists.should.be.true
					done()
			it "should reject invalid oids", ->
				# Completely invalid oid.
				(->
					repo.exists "!!!"
				).should.throw()

				# Valid oid, but full 40 char oids are required for exists()
				(->
					repo.exists "abcd123"
				).should.throw()
		describe "#object()", ->
			it "works for full OID", (done) ->
				repo.object "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
			it "works for shortened OID", (done) ->
				repo.object "1f4425ce2a", (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
		describe "#commit()", ->
			it "works", (done) ->
				repo.commit "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14", (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
			it "fails for objects that aren't a commit", (done) ->
				repo.commit fixtures.projectRepo.secondCommit.wscriptBlob, (err, obj) ->
					should.exist err
					done()
		describe "#tree()", ->
			it "works", (done) ->
				repo.tree fixtures.projectRepo.secondCommit.tree, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Tree
					done()
			it "fails for objects that aren't a tree", (done) ->
				repo.tree fixtures.projectRepo.secondCommit.id, (err, obj) ->
					should.exist err
					done()
		describe "#blob()", ->
			it "works", (done) ->
				repo.blob fixtures.projectRepo.secondCommit.wscriptBlob, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Blob
					done()
			it "fails for objects that aren't a blob", (done) ->
				repo.blob fixtures.projectRepo.secondCommit.id, (err, obj) ->
					should.exist err
					done()
