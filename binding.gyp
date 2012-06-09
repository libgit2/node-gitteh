{
	'targets': [
		{
			'target_name': 'gitteh',
			'sources': [
				'src/gitteh.cc',
				'src/signature.cc',
				'src/repository.cc',
				'src/baton.cc',
				'src/commit.cc',
				'src/tree.cc',
				'src/blob.cc',
				'src/tag.cc',
				'src/remote.cc',
				'src/index.cc',
			],
			'todosources': [
				'src/index_entry.cc',
				'src/tag.cc',
				'src/rev_walker.cc',
				'src/ref.cc',
			],

			'include_dirs': [
				'deps/v8-convert',
				'deps/libgit2/include'
			],

			'libraries': [
				'-L<!(pwd)/deps/libgit2/build',
				'-lgit2'
			],

			'cflags': [
				'-Wall'
			],

			'ldflags': [
				'-Wl,-rpath,\$$ORIGIN/../../deps/libgit2/build'
			]
		}
	]
}
