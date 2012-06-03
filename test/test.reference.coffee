path = require "path"
should = require "should"
fs = require "fs"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

describe "Reference", ->
	repo = null
	ref = null
	headFile = null

	describe "Using the project repo...", ->
		it "can find the HEAD sym reference", (done) ->
			gitteh.openRepository fixtures.projectRepo.path, (err, _repo) ->
				repo = _repo

				# We'll need this later.
				headPath = path.join repo.path, "HEAD"
				headFile = fs.readFileSync(headPath, "utf8")[5..-2]

				repo.ref "HEAD", false, (err, ref_) ->
					should.not.exist err
					ref = ref_
					ref.should.be.an.instanceof gitteh.Reference
					done()
	describe "The HEAD sym reference", ->
		describe "#name", ->
			it "should be HEAD", -> ref.name.should.equal "HEAD"
			it "should be immutable", -> utils.checkImmutable ref, "name"
		describe "#target", ->
			it "should point to .git/HEAD", ->
				ref.target.should.equal headFile
			it "should be immutable", -> utils.checkImmutable ref, "target"
		describe "#direct", ->
			it "should be false", -> ref.direct.should.be.false
			it "should be immutable", -> utils.checkImmutable ref, "direct"
		describe "#packed", ->
			it "should be false", -> ref.packed.should.be.false
			it "should be immutable", -> utils.checkImmutable ref, "packed"
	describe "Resolving HEAD", ->
		it "gives us correct resolved direct ref", ->
			repo.ref "HEAD", true, (err, ref) ->
				should.not.exist err
				ref.name.should.equal headFile
