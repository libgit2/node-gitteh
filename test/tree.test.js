var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
	profiler = require("profiler"),
	fixtureValues = require("./fixtures/values"),
	helpers = require("./fixtures/helpers.js");

var DIRECTORY_ATTRIBUTE = helpers.fromOctal(40000);

var repo = new gitteh.Repository(fixtureValues.REPO_PATH);

var createTreeTestContext = function(treeFixture) {
	var context = {
		topic: function() {
			return repo.getTree(treeFixture.id);
		},
		
		"gives us a Tree": function(tree) {
			assert.isTrue(!!tree);
		},
		
		"with correct id": function(tree) {
			assert.equal(tree.id, treeFixture.id);
		}
	};
	
	// Run assertions on the contents of tree.
	var createEntriesChecks = function(entriesFixture, path) {
		var entriesContext = {
			topic: function(tree) {
				return tree.entries;
			},
			
			"has correct number of entries": function(entries) {
				assert.length(entries, entriesFixture.length);
			}
		};
		
		context[(path == "") ? "- root entries" : ("- tree " + path)] = entriesContext;

		for(var i = 0; i < entriesFixture.length; i++) {
			entriesContext["- entry " + entriesFixture[i].filename] = (function(i) {
				var theContext = {
					topic: function(entries) {
						return entries[i];
					},
					
					"has correct name": function(entry) {
						assert.equal(entry.filename, entriesFixture[i].filename);
					},
					
					"has correct attributes": function(entry) {
						assert.equal(helpers.toOctal(entry.attributes), entriesFixture[i].attributes);
					},
					
					"has correct id": function(entry) {
						assert.equal(entry.id, entriesFixture[i].id);
					}
				};
				
				// We're dealing with a folder here. Let's recurse into it and check it out.
				if(entriesFixture[i].attributes == DIRECTORY_ATTRIBUTE) {
					createEntriesChecks(entriesFixture[i].entries, path + "/" + entriesFixture[i].filename);
				}
				
				return theContext;				
			})(i);
		}
		
		
	};
	
	createEntriesChecks(treeFixture.entries, "");
	
	return context;
}

vows.describe("Tree").addBatch({
	"First tree": createTreeTestContext(fixtureValues.FIRST_TREE),
	"Second tree": createTreeTestContext(fixtureValues.SECOND_TREE),
	"Third tree": createTreeTestContext(fixtureValues.THIRD_TREE),
	"Fourth tree": createTreeTestContext(fixtureValues.FOURTH_TREE),
	"Fifth tree": createTreeTestContext(fixtureValues.FIFTH_TREE),
}).export(module);
