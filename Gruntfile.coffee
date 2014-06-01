# JS Hint options
JSHINT_NODE =
  node: true,
  strict: false

module.exports = (grunt) ->

  # Project configuration.
  grunt.initConfig
    pkg: grunt.file.readJSON 'package.json'
    pkgFile: 'package.json'

    files:
      server: ['lib/**/*.js']

    coffee:
      glob_to_multiple:
        expand: true
        flatten: true
        cwd: '.'
        src: ['src/*.coffee']
        dest: 'lib/'
        ext: '.js'

    # JSHint options
    # http://www.jshint.com/options/
    jshint:
      server:
        files:
          src: '<%= files.server %>'
        options: JSHINT_NODE

      options:
        quotmark: 'single'
        bitwise: true
        freeze: true
        indent: 2
        camelcase: true
        strict: true
        trailing: true
        curly: true
        eqeqeq: true
        immed: true
        latedef: true
        newcap: true
        noempty: true
        unused: true
        noarg: true
        sub: true
        undef: true
        maxdepth: 4
        maxstatements: 100
        maxcomplexity: 100
        maxlen: 100
        globals: {}

    jscs:
      server: files: src: '<%= files.server %>'
      options:
        config: '.jscs.json'

  require('load-grunt-tasks') grunt

  grunt.registerTask 'build', ['coffee']
  grunt.registerTask 'default', ['build']
  grunt.registerTask 'lint', ['jshint', 'jscs']
