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

			'dependencies': [
				'<(module_root_dir)/deps/libgit2.gyp:libgit2'
			],

			'ldflags': [
				'-Wl,-rpath,\$$ORIGIN/Release'
			],

			'include_dirs': [
				'deps/v8-convert',
				"<!(node -e \"require('nan')\")"
			],

			'cflags': [
				'-Wall'
			],

			'libraries': [
				'-L<!(pwd)/build/Release',
				'-lgit2'
			],

			'conditions': [
				['OS=="mac"', {
					'xcode_settings': {
						'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
					}
				}]
			]
		}
	]
}
