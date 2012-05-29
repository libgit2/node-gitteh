path = require "path"
should = require "should"
crypto = require "crypto"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

secondCommit = fixtures.projectRepo.secondCommit

describe "Blob", ->
	repo = null
	blob = null

	describe "Using the project repo...", ->
		it "can find the wscript blob (#{secondCommit.wscriptBlob}) file in second commit", (done) ->
			gitteh.openRepository fixtures.projectRepo.path, (err, _repo) ->
				repo = _repo
				repo.blob secondCommit.wscriptBlob, (err, _blob) ->
					should.not.exist err
					blob = _blob
					blob.should.be.an.instanceof gitteh.Blob
					done()
		describe "#id", ->
			it "is correct", ->
				blob.id.should.equal secondCommit.wscriptBlob
			it "is immutable", -> utils.checkImmutable blob, "id"
		describe "#data", ->
			it "is a Buffer", ->
				blob.data.should.be.an.instanceof Buffer
			it "has correct contents", ->
				shasum = crypto.createHash "sha1"
				shasum.update "blob #{blob.data.length}\u0000"
				shasum.update blob.data.toString("binary")
				shasum.digest("hex").should.equal blob.id
			it "is immutable", -> utils.checkImmutable blob, "data"