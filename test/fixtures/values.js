module.exports.REPO_PATH = require("path").join(__dirname, "gitrepo");

module.exports.WORKING_DIR = {
	gitDirectory: require("path").join(__dirname, "workingdir", "dotgit"),
	workTree: require("path").join(__dirname, "workingdir")
};

module.exports.FIRST_COMMIT = {
	id: "ca1f3314acd24c4551da8c26adaf562272db1d19",
	message: "First commit.\n",
	authorName: "Sam Day",
	authorEmail: "sam.c.day@gmail.com",
	committerName: "Sam Day",
	committerEmail: "sam.c.day@gmail.com",
	time: new Date("Mon Feb 28 14:59:45 2011 +1000"),
	parents: [],
	tree: "3020f22e8b22650f524523710f3c211f214f8e75"
};

module.exports.SECOND_COMMIT = {
	id: "91ef0fd7ea773461d4d57dc1e5675a9bc83d6081",
	message: "Second commit, woohoo\\!\n",
	authorName: "Sam Day",
	authorEmail: "sam.c.day@gmail.com",
	committerName: "Sam Day",
	committerEmail: "sam.c.day@gmail.com",
	time: new Date("Wed Mar 2 08:57:26 2011 +1000"),
	parents: ["ca1f3314acd24c4551da8c26adaf562272db1d19"],
	tree: "6a670a961ede0d6965e5326797e074d26ecee235"
};

module.exports.THIRD_COMMIT = {
	id: "46d47fb28223a7f29df5d2072eb05e380985a477",
	message: "Third commit.\nThis one spans\na few lines.\n",
	authorName: "Sam Day",
	authorEmail: "sam.c.day@gmail.com",
	committerName: "Sam Day",
	committerEmail: "sam.c.day@gmail.com",
	time: new Date("Wed Mar 2 09:31:08 2011 +1000"),
	parents: ["91ef0fd7ea773461d4d57dc1e5675a9bc83d6081"],
	tree: "6cd2ee6530e7f104040569bd7bf516f90e86e747"
};

module.exports.FOURTH_COMMIT = {
	id: "bae21d9853dc927b3784420144afd9082d768024",
	message: "Fourth commit.\n",
	authorName: "Sam Day",
	authorEmail: "sam.c.day@gmail.com",
	committerName: "Sam Day",
	committerEmail: "sam.c.day@gmail.com",
	time: new Date("Wed Mar 2 09:31:45 2011 +1000"),
	parents: ["91ef0fd7ea773461d4d57dc1e5675a9bc83d6081"],
	tree: "e51d4f52d0b38a9052c6cebaa1b49a0b75e8d82e"
};

module.exports.FIFTH_COMMIT = {
	id: "38042ca303189a2e6955b92d53aab7d642cc82c2",
	message: "Merge branch 'test'\n",
	authorName: "Sam Day",
	authorEmail: "sam.c.day@gmail.com",
	committerName: "Sam Day",
	committerEmail: "sam.c.day@gmail.com",
	time: new Date("Wed Mar 2 09:31:57 2011 +1000"),
	parents: ["46d47fb28223a7f29df5d2072eb05e380985a477", "bae21d9853dc927b3784420144afd9082d768024"],
	tree: "98695ebf529d456e0e2cbf16ff6e06b0bfe3e843"
};

module.exports.FIRST_TREE = {
	id: "3020f22e8b22650f524523710f3c211f214f8e75",
	
	entries: [
		{
			id: "47ee7698c336ba5b163c193ae6309f0a7d7e9662",
			filename: "file.txt",
			attributes: 100644
		}
	]
};

module.exports.SECOND_TREE = {
	id: "6a670a961ede0d6965e5326797e074d26ecee235",
	
	entries: [
		{
			id: "47ee7698c336ba5b163c193ae6309f0a7d7e9662",
			filename: "file.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "second.txt",
			attributes: 100644
		}
	]
};

module.exports.THIRD_TREE = {
	id: "6cd2ee6530e7f104040569bd7bf516f90e86e747",
	
	entries: [
		{
			id: "47ee7698c336ba5b163c193ae6309f0a7d7e9662",
			filename: "file.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "second.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "third.txt",
			attributes: 100644
		}
	]
};

module.exports.FOURTH_TREE = {
	id: "e51d4f52d0b38a9052c6cebaa1b49a0b75e8d82e",
	
	entries: [
		{
			id: "47ee7698c336ba5b163c193ae6309f0a7d7e9662",
			filename: "file.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "fourth.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "second.txt",
			attributes: 100644
		}
	]
};

module.exports.FIFTH_TREE = {
	id: "98695ebf529d456e0e2cbf16ff6e06b0bfe3e843",
	
	entries: [
		{
			id: "47ee7698c336ba5b163c193ae6309f0a7d7e9662",
			filename: "file.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "fourth.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "second.txt",
			attributes: 100644
		},
		{
			id: "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391",
			filename: "third.txt",
			attributes: 100644
		}
	]
};

module.exports.TEST_TAG = {
	id: "dfa48f906451815913215afb4ef58321c33824e7",
	name: "test_tag",
	message: "My test tag.\n",
	tagger: {
		name: "Sam Day",
		email: "sam.c.day@gmail.com",
		time: "Wed Mar 2 2011 14:43:28 2011 +1000"
	},
	targetId: module.exports.FIFTH_COMMIT.id,
	targetType: "commit",
	rawBody: "object 38042ca303189a2e6955b92d53aab7d642cc82c2\ntype commit\ntag test_tag\ntagger Sam Day <sam.c.day@gmail.com> 1299041008 +1000\n\nMy test tag.\n"
};

module.exports.EMPTY_BLOB = "e69de29bb2d1d6434b8b29ae775ad8c2e48c5391";

module.exports.LATEST_COMMIT = module.exports.FIFTH_COMMIT;