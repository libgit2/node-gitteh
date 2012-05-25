{
	'targets': [
		{
			'target_name': 'gitteh',
			'sources': [
				'src/gitteh.cc',
				'src/git_object.cc',
				'src/signature.cc',
				'src/repository.cc',
				'src/object_cache.cc',
				'src/baton.cc',
				'src/commit.cc',
			],
			'todosources': [
				'src/tree.cc',
				'src/index.cc',
				'src/index_entry.cc',
				'src/tag.cc',
				'src/rev_walker.cc',
				'src/ref.cc',
				'src/blob.cc'
			],

			'include_dirs': [
				'deps/v8-convert'
			],

			'cflags': [
				'-Wall'
			],

			'conditions': [
				[ 'OS=="windows"', {

				}, {
					'libraries': [
						'<!@(pkg-config --libs libgit2)'
					]
				}],
				['OS=="mac"', {
					# cflags on OS X are stupid and have to be defined like this
					'xcode_settings': {
					'OTHER_CFLAGS': [
						'-Wall',
						'<!@(pkg-config --cflags libgit2)'
					]
				  }
				}, {
					'cflags': [
						'<!@(pkg-config --cflags libgit2)'
					],
				}]
			]
		}
	]
}
