import Options
from os import unlink, symlink, popen
from os.path import exists 

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options("compiler_cc")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("compiler_cc")
  conf.check_tool("node_addon")

  if not conf.check(lib='bsd', libpath=['/usr/lib'], uselib_store='LIBBSD'):
    conf.fatal("Cannot find bsd libraries (used for arc4random).")

def build(bld):
  bcryptnode = bld.new_task_gen("cxx", "shlib", "node_addon")
  bcryptnode.target = "bcrypt_node"
  bcryptnode.source = """
    src/blowfish.cc
    src/bcrypt.cc
    src/bcrypt_node.cc
  """
  bcryptnode.includes = """
    /usr/includes/bsd/
  """
  bcryptnode.uselib = 'LIBBSD'

def shutdown():
  # HACK to get bcrypt.node out of build directory.
  # better way to do this?
  if Options.commands['clean']:
    if exists('bcrypt_node.node'): unlink('bcrypt_node.node')
  else:
    if exists('build/default/bcrypt_node.node') and not exists('bcrypt_node.node'):
      symlink('build/default/bcrypt_node.node', 'bcrypt_node.node')
