{
	'targets': [
		{
			'target_name': 'gitteh',
			'sources': [
				'src/gitteh.cc',
				'src/repository.cc',
			],
			'todosources': [
				'src/commit.cc',
				'src/tree.cc',
				'src/index.cc',
				'src/index_entry.cc',
				'src/tag.cc',
				'src/rev_walker.cc',
				'src/ref.cc',
				'src/blob.cc'
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
