# -*- mode: python -*-

import Options, Utils, sys, re, os

srcdir = "."
blddir = "build"
VERSION = "0.0.1"
node_version = os.popen("node --version").read()

def set_options(opt):
  opt.tool_options("compiler_cxx")

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("node_addon")
  o = Options.options

  nodepath = 'NODE_PATH' in os.environ and os.environ['NODE_PATH'] or None

  libpath = ['/lib', '/usr/lib', '/usr/local/lib', '/opt/local/lib', '/usr/sfw/lib']
  if nodepath:
    libpath.append(nodepath)
  includes = ['/usr/include', '/usr/includes', '/usr/local/includes', '/opt/local/includes', '/usr/sfw/lib'];

  libssl = conf.check(lib="ssl",
          header_name='openssl/rand.h',
          includes=includes,
          libpath=libpath,
          mandatory=True,
          uselib_store='OPENSSL')

  if sys.platform == "cygwin":
    libcrypto = conf.check(lib="crypto",
            includes=includes,
            libpath=libpath,
            uselib_store='CRYPTO')
    libz = conf.check(lib="z",
            includes=includes,
            libpath=libpath,
            uselib_store='Z')

def build(bld):
  bcryptnode = bld.new_task_gen("cxx", "shlib", "node_addon")
  cxxflags = [ "-O3" ]
  if not node_version.startswith("v0.4"):
    cxxflags.append("-DEV_MULTIPLICITY=1")
  bcryptnode.cxxflags = cxxflags
  bcryptnode.target = "bcrypt_lib"
  bcryptnode.source = """
    src/blowfish.cc
    src/bcrypt.cc
    src/bcrypt_node.cc
  """
  uselib = "OPENSSL"
  if sys.platform == "cygwin":
    uselib += " CRYPTO Z"
  bcryptnode.uselib = uselib

def test(t):
  Utils.exec_command('make test')
