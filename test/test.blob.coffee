path = require "path"
should = require "should"
crypto = require "crypto"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

secondCommit = fixtures.testRepo.secondCommit

describe "Blob", ->
	repo = null
	blob = null

	describe "Using the test repo...", ->
		it "can find the README blob (#{secondCommit.readmeBlob}) file in second commit", (done) ->
			gitteh.openRepository fixtures.testRepo.path, (err, _repo) ->
				repo = _repo
				repo.blob secondCommit.readmeBlob, (err, _blob) ->
					should.not.exist err
					blob = _blob
					blob.should.be.an.instanceof gitteh.Blob
					done()
		describe "#id", ->
			it "is correct", ->
				blob.id.should.equal secondCommit.readmeBlob
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
