path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

describe "Reference", ->
	repo = null
	ref = null

	describe "Using the project repo...", ->
		it "can find the HEAD sym reference", (done) ->
			gitteh.openRepository fixtures.projectRepo.path, (err, _repo) ->
				repo = _repo

				repo.ref "HEAD", false, (err, ref_) ->
					should.not.exist err
					ref = ref_
					# ref.should.be.an.instanceof gitteh.Reference
					done()
	describe "The HEAD sym reference", ->
		describe "#direct", ->
			it "should be false", ->
				ref.direct.should.be.false
