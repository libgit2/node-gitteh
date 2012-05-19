{
	'targets': [
		{
			'target_name': 'gitteh',
			'sources': [
				'src/gitteh.cc',
			],
			'todosources': [
				'src/commit.cc',
				'src/tree.cc',
				'src/repository.cc',
				'src/index.cc',
				'src/index_entry.cc',
				'src/tag.cc',
				'src/rev_walker.cc',
				'src/ref.cc',
				'src/blob.cc'
			],

			'conditions': [
				[ 'OS=="linux"', {
					'libraries': [
						'<!@(pkg-config --libs libgit2)'
					],
					'cflags': [
						'<!@(pkg-config --cflags libgit2)'
					]
				}]
			]
		}
	]
}
