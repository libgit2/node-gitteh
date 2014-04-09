module.exports = function(grunt) {

  /**
   * Run tests
   *
   * grunt test
   * grunt test:unit
   */
  grunt.registerMultiTask('test', 'Run tests.', function() {
    var specDone = this.async();
    var path = require('path');

    // UNIT tests or TASK tests
    grunt.task.run([this.data]);
    specDone();
  });
};
