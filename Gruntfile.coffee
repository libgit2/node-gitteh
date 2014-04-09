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

    test:
      unit: 'simplemocha:unit'

    simplemocha:
      options:
        globals: ['should']
        ui: 'bdd'
        reporter: 'dot'
      unit:
        src: [
          'test/**/*.coffee'
        ]

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

    # CoffeeLint options
    # http://www.coffeelint.org/#options
    coffeelint:
      unittests: files: src: ['test/**/*.coffee']
      options:
        max_line_length:
          value: 100

    jscs:
      server: files: src: '<%= files.server %>'
      options:
        config: '.jscs.json'

  grunt.loadTasks 'tasks'
  require('load-grunt-tasks') grunt

  grunt.registerTask 'build', ['coffee']
  grunt.registerTask 'default', ['build', 'test']
  grunt.registerTask 'lint', ['jshint', 'jscs', 'coffeelint']
