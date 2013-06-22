path = require "path"

exports.projectRepo =
	path: path.join __dirname, "../"
	gitPath: path.join __dirname, "..", ".git/"

	firstCommit:
		id: "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14"
	secondCommit:
		id: "8a916d5fbce49f5780668a1ee780e0ef2e89360f"
		tree: "aa41780f2129bf03cce1a3eeadc78db47f83d9ad"
		parent: "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14"
		message: "Stuff."
		wscriptBlob: "70cefa94cfaa928cd2b601d38be7ea221f0e219e"

exports.testRepo =
	path: path.join __dirname, "testrepo.git/"
	firstCommit:
		id: "8496071c1b46c854b31185ea97743be6a8774479"
		message: "testing"
	secondCommit:
		author:
			name: "Scott Chacon"
			email: "schacon@gmail.com"
			time: "Tue, 11 May 2010 20:38:42 GMT"
			offset: -420
		id: "5b5b025afb0b4c913b4c338a42934a3863bf3644"
		tree: "f60079018b664e4e79329a7ef9559c8d9e0378d1"
		parent: "8496071c1b46c854b31185ea97743be6a8774479"
		message: "another commit"
		readmeBlob: "1385f264afb75a56a5bec74243be9b367ba4ca08"
