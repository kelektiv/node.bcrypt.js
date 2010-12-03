import Options, Utils, sys
from os import unlink, symlink, popen
from os.path import exists, islink

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

  if sys.platform != 'darwin' and not conf.check(lib='bsd', libpath=['/usr/lib'], uselib_store='LIBBSD'):
    conf.fatal("Cannot find bsd libraries (used for arc4random).")

def build(bld):
  bcryptnode = bld.new_task_gen("cxx", "shlib", "node_addon")
  bcryptnode.target = "bcrypt_lib"
  bcryptnode.source = """
    src/blowfish.cc
    src/bcrypt.cc
    src/bcrypt_node.cc
  """

  if sys.platform != 'darwin':
    bcryptnode.includes = """
      /usr/includes/bsd/
    """
    bcryptnode.uselib = 'LIBBSD'

def test(t):
  Utils.exec_command('nodeunit test')

def shutdown():
  t = 'bcrypt_lib.node'
  if Options.commands['clean']:
    if exists(t): unlink(t)
  if Options.commands['build']:
    if exists('build/default/' + t) and not exists(t):
      symlink('build/default/' + t, t)
