srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool('compiler_cxx')
  conf.check_tool('node_addon')
  
  if not conf.check_cxx(header_name="git2.h"):
  	conf.fatal("git2.h header was not found. Make sure you installed libgit2 correctly.")
  	
  if not conf.check_cxx(lib="git2"):
  	conf.fatal("libgit2 is not installed on your system.")

def build(bld):
  obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
  obj.target = 'libgit2_bindings'
  obj.source = 'src/libgit2_bindings.cc'
  obj.lib = 'git2'
