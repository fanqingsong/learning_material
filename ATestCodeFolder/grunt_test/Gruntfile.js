module.exports = function(grunt) {
	grunt.initConfig({
		pkg: grunt.file.readJSON("package.json"),
		uglify: {
			options:{
				stripBanners: true,
				banner: '/*! <%=pkg.name%>-<%pkg.version%>.js <%=grunt.template.today("yyyy-mm-dd")%> */\n'
			},
		    my_target: {
		      files: {
		        'build/test.min.js': ['src/test.js'],
		        'build/dijstra_core.min.js': ['src/dijstra_core.js'],
		      }
		    }
		},
		jshint:{
			build: [ 'Gruntfile.js', 'src/test.js' ],
			options: {
				jshintrc: '.jshintrc'
			}
		}
	});

	grunt.loadNpmTasks('grunt-contrib-uglify');

	grunt.loadNpmTasks('grunt-contrib-jshint');

	grunt.registerTask('default', ['jshint', 'uglify']);
};