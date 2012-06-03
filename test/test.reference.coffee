path = require "path"
should = require "should"
fs = require "fs"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

{secondCommit} = fixtures.projectRepo

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
	describe "Creating a direct reference to second commit", ->
		newRef = null
		after (cb) ->
			fs.unlink path.join(repo.path, "refs", "heads", "testref"), cb
		it "works", (done) ->
			repo.createReference "refs/heads/testref", secondCommit.id, (err, _ref) ->
				newRef = _ref
				should.not.exist err
				newRef.should.be.an.instanceof gitteh.Reference
				done()
		it "should have correct data", ->
			newRef.name.should.equal "refs/heads/testref"
			newRef.target.should.equal secondCommit.id
		it "should fail when trying to create ref with same name again", (done) ->
			repo.createReference "refs/heads/testref", secondCommit.id, (err, _ref) ->
				should.exist err
				err.should.be.an.instanceof Error
				done()