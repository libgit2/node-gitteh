path = require "path"
should = require "should"
gitteh = require "../lib/gitteh"
utils = require "./utils"
fixtures = require "./fixtures"

secondCommit = fixtures.projectRepo.secondCommit

###
describe "Tree", ->
	repo = null
	tree = null

	describe "Using the project repo...", ->
		it "can find second commit tree (#{secondCommit.tree})", (done) ->
			gitteh.openRepository fixtures.projectRepo.path, (err, _repo) ->
				repo = _repo

				# Epic WTF : console.log here prevents an obscure failure in C++
				# land. Look into this ASAP!
				console.log " "

				repo.tree secondCommit.tree, (err, _tree) ->
					tree = _tree
					should.not.exist err
					tree.should.be.an.instanceof gitteh.Tree
					done()
	describe "The second project commit tree...", ->
		it "has correct id", ->
			tree.id.should.equal secondCommit.tree
###