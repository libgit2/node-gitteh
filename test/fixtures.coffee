path = require "path"

exports.projectRepo = 
	path: path.join __dirname, "../"
	gitPath: path.join __dirname, "..", ".git/"

	secondCommit:
		id: "8a916d5fbce49f5780668a1ee780e0ef2e89360f"
		tree: "aa41780f2129bf03cce1a3eeadc78db47f83d9ad"
		parent: "1f4425ce2a14f21b96b9c8dde5bcfd3733467b14"
		message: "Stuff."
		wscriptBlob: "70cefa94cfaa928cd2b601d38be7ea221f0e219e"