var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("../build/default/gitteh"),
	path = require("path"),
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
	/*"First tree": createTreeTestContext(fixtureValues.FIRST_TREE),
	"Second tree": createTreeTestContext(fixtureValues.SECOND_TREE),
	"Third tree": createTreeTestContext(fixtureValues.THIRD_TREE),
	"Fourth tree": createTreeTestContext(fixtureValues.FOURTH_TREE),
	"Fifth tree": createTreeTestContext(fixtureValues.FIFTH_TREE),
	
	"Retrieving tree entries by name": {
		topic: function() {
			var tree = repo.getTree(fixtureValues.FIRST_TREE.id);
			this.context.tree = tree;
			return tree.getByName(fixtureValues.FIRST_TREE.entries[0].filename);
		},
		
		"gives us the correct entry": function(entry) {
			assert.equal(entry.filename, fixtureValues.FIRST_TREE.entries[0].filename);
			assert.equal(entry.id, fixtureValues.FIRST_TREE.entries[0].id);	
		},
		
		"identical to getting it via index": function(entry) {
			assert.isTrue(entry === this.context.tree.entries[0]);
		}
	}*/
	
	"Creating a new Tree": {
		topic: function() {
			return repo.createTree();
		},
		
		"gives us a new Tree": function(tree) {
			assert.isTrue(!!tree);
		},
		
		"with correct identity": function(tree) {
			assert.isNull(tree.id);
			assert.length(tree.entries, 0);
		},
		
		"- adding an entry": {
			topic: function(tree) {
				tree.addEntry(fixtureValues.EMPTY_BLOB, "test", helpers.fromOctal(100644));
				return tree;
			},
			
			"adds to tree *entries* correctly": function(tree) {
				assert.length(tree.entries, 1);
			},
			
			"entry has correct values": function(tree) {
				assert.equal(tree.entries[0].id, fixtureValues.EMPTY_BLOB);
				assert.equal(tree.entries[0].attributes, helpers.fromOctal(100644));
				assert.equal(tree.entries[0].filename, "test");
			},
			
			"- saving": {
				topic: function(tree) {
					tree.save();
					return tree;
				},
				
				"updates id correctly": function(tree) {
					assert.equal(tree.id, "f05af273ba36fe5176e5eaab349661a56b3d27a0");
				},
				
				"this tree is now available from Repository": function(tree) {
					assert.isTrue(tree === repo.getTree("f05af273ba36fe5176e5eaab349661a56b3d27a0"));
				}
			}
		}
	}
}).export(module);
