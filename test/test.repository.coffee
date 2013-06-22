path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
fixtures = require "./fixtures"

describe "Repository", ->
	describe "Using the test repo...", ->
		repo = null
		it "opens correctly", (done) ->
			gitteh.openRepository fixtures.testRepo.path, (err, _repo) ->
				repo = _repo
				repo.should.be.an.instanceof gitteh.Repository
				done()
		describe "#bare", ->
			it "should be true for this repo", ->
				repo.bare.should.be.true
			it "should be immutable", ->
				repo.bare = 123
				repo.bare.should.be.true
				delete repo.path
				repo.bare.should.be.true
		describe "#path", ->
			it "should point to #{fixtures.testRepo.path}", ->
				repo.path.should.equal fixtures.testRepo.path
			it "should be immutable", ->
				repo.path = "foo"
				repo.path.should.equal fixtures.testRepo.path
				delete repo.path
				repo.path.should.equal fixtures.testRepo.path
		describe "#workingDirectory", ->
			it "should be null", ->
				should.not.exist repo.workingDirectory
		describe "#exists()", ->
			it "should return true for first commit in repo :)", (done) ->
				repo.exists fixtures.testRepo.firstCommit.id, (err, exists) ->
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
				repo.object fixtures.testRepo.secondCommit.id, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
			it "works for shortened OID", (done) ->
				repo.object fixtures.testRepo.secondCommit.id[..8], (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
		describe "#commit()", ->
			it "works", (done) ->
				repo.commit fixtures.testRepo.secondCommit.id, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Commit
					done()
			it "fails for objects that aren't a commit", (done) ->
				repo.commit fixtures.testRepo.secondCommit.readmeBlob, (err, obj) ->
					should.exist err
					done()
		describe "#tree()", ->
			it "works", (done) ->
				repo.tree fixtures.testRepo.secondCommit.tree, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Tree
					done()
			it "fails for objects that aren't a tree", (done) ->
				repo.tree fixtures.testRepo.secondCommit.id, (err, obj) ->
					should.exist err
					done()
		describe "#blob()", ->
			it "works", (done) ->
				repo.blob fixtures.testRepo.secondCommit.readmeBlob, (err, obj) ->
					should.not.exist err
					obj.should.be.an.instanceof gitteh.Blob
					done()
			it "fails for objects that aren't a blob", (done) ->
				repo.blob fixtures.testRepo.secondCommit.id, (err, obj) ->
					should.exist err
					done()
