import Options
import os
from subprocess import Popen
from os.path import exists, abspath

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
	opt.tool_options('compiler_cxx')
	
	opt.add_option("--debug",
					action = "store_true",
					default = False,
					help = "Compile gitteh and libgit2 (if using bundled version) with debug flags.")

def configure_libgit2(ctx):
	o = Options.options
	
	if not ctx.check(lib = "git2", uselib_store = "GIT2"):
		# Checkout libgit2 submodule if it isn't already.
		if exists(".git"): 
			if not exists("vendor/libgit2") or not os.listdir("vendor/libgit2"):
				print "Checking out libgit2 submodule."
				if Popen("{0} submodule update --init".format(ctx.env.GIT), shell = True).wait() != 0:
					ctx.fatal("Couldn't initialize libgit2 submodule.")
		
		print "Configuring libgit2..."
		command = "./waf configure"
		
		if(o.debug):
			command = command + " --debug"

		if Popen(command, shell = True, cwd = "vendor/libgit2").wait() != 0:
			ctx.fatal("Libgit2 failed to configure.")

		ctx.env.LIBPATH_GIT2 = abspath("vendor/libgit2/build/shared")
		ctx.env.LIB_GIT2 = "git2"
		ctx.env.RPATH_GIT2 = abspath("vendor/libgit2/build/shared")
		ctx.env.internalLibgit2 = True

		ctx.env.append_value("CPPPATH_GIT2", [abspath("vendor/libgit2/include")])

	else:
		ctx.env.internalLibgit2 = False

def build_libgit2(bld):
	if Popen("./waf build", cwd = "vendor/libgit2", shell = True).wait() != 0:
		# TODO: is there some way of crashing out with an error message from build
		# context?
		#bld.fatal("Errors building libgit2.") 
		pass

def clean_libgit2(bld):
	Popen("./waf clean", cwd = "vendor/libgit2", shell = True).wait()

def configure(ctx):
	ctx.check_tool('compiler_cxx')
	ctx.check_tool('node_addon')

	o = Options.options

	ctx.find_program("git", var="GIT", mandatory = True)

	configure_libgit2(ctx)
		
	if o.debug:
		ctx.env.append_value("CXXFLAGS", ["-ggdb", "-O0", "-Wall"]) 

def build(ctx):
	if(ctx.env.internalLibgit2):
		build_libgit2(ctx)

	obj = ctx.new_task_gen('cxx', 'shlib', 'node_addon')
	obj.target = 'gitteh'
	obj.source = 'src/gitteh.cc src/commit.cc src/tree.cc src/tree_entry.cc src/repository.cc src/index.cc src/index_entry.cc src/tag.cc src/rev_walker.cc src/ref.cc src/blob.cc' 
	obj.uselib = 'GIT2'