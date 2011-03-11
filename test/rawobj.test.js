/*
 * The MIT License
 *
 * Copyright (c) 2010 Sam Day
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
 var vows = require("vows"),
	assert = require("assert"),
	gitteh = require("gitteh"),
	path = require("path"),
	fixtureValues = require("./fixtures/values");

var repo = gitteh.openRepository(fixtureValues.REPO_PATH);
vows.describe("RawObj").addBatch({
	"Opening a raw object": {
		topic: function() {
			repo.getRawObject(fixtureValues.TEST_TAG.id, this.callback);
		},
		
		"gives us an object": function(obj) {
			assert.isTrue(!!obj);
		},
		
		"with the correct *id*": function(obj) {
			assert.equal(obj.id, fixtureValues.TEST_TAG.id);
		},

		"with the correct *type*": function(obj) {
			assert.equal(obj.type, "tag");
		},
		
		"*data* is a Buffer": function(obj) {
			assert.instanceOf(obj.data, Buffer);
		},
		
		"*data* is correct length": function(obj) {
			assert.equal(obj.data.length, fixtureValues.TEST_TAG.rawBody.length);
		},
		
		"*data* is correct content": function(obj) {
			assert.equal(obj.data.toString(), fixtureValues.TEST_TAG.rawBody);
		},
		
		"object is *not* part of object graph": function(obj) {
			assert.isFalse(obj === repo.getRawObject(fixtureValues.TEST_TAG.id));
		}
	},
	
	"Creating a new raw object": {
		topic: function() {
			repo.createRawObject(this.callback);
		},
		
		"gives us an identity raw object": function(rawobj) {
			assert.isNull(rawobj.id);
			assert.equal(rawobj.type, "");
			assert.isNull(rawobj.data);
		},
		
		"id is immutable": function(rawobj) {
			rawobj.id = "foo";
			assert.isNull(rawobj.id);
		},
		
		"id cannot be deleted": function(rawobj) {
			delete rawobj.id;
			assert.isNull(rawobj.id);
		},
		
		"saving with correct data works": function(rawobj) {
			rawobj.type = "blob";
			rawobj.data = new Buffer("Hello world!");
			rawobj.save();
		},
		
		"object has correct id": function(rawobj) {
			assert.equal(rawobj.id, "6769dd60bdf536a83c9353272157893043e9f7d0");
		},
		
		"data saved correctly": function(rawobj) {
			var actual = repo.getRawObject(rawobj.id);
			assert.equal(rawobj.id, actual.id);
			assert.equal(rawobj.type, actual.type);
			assert.equal(rawobj.data.toString(), actual.data.toString());
		},
		
		"object is not part of graph": function(rawobj) {
			assert.isFalse(rawobj === repo.getRawObject(rawobj.id));
		}
	}
}).export(module);
