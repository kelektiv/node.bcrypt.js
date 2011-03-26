# -*- mode: python -*-

import Options, Utils, sys
from os import unlink, symlink, popen
from os.path import exists, islink

srcdir = "."
blddir = "build"
VERSION = "0.0.1"

def set_options(opt):
  opt.tool_options("compiler_cxx")
  opt.tool_options("compiler_cc")
  opt.add_option( '--openssl-libpath'
                , action='store'
                , default=False
                , help="A directory to search for the OpenSSL libraries"
                , dest='openssl_libpath'
                )
  opt.add_option( '--openssl-includes'
                , action='store'
                , default=False
                , help='A directory to search for the OpenSSL includes'
                , dest='openssl_includes'
                )

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool("compiler_cc")
  conf.check_tool("node_addon")
  o = Options.options

  if o.openssl_libpath: 
    openssl_libpath = [o.openssl_libpath]
  elif not sys.platform.startswith('win32'):
    openssl_libpath = ['/usr/lib', '/usr/local/lib', '/opt/local/lib', '/usr/sfw/lib']
  else:
    openssl_libpath = [normpath(join(cwd, '../openssl'))]

  if o.openssl_includes: 
    openssl_includes = [o.openssl_includes]
  elif not sys.platform.startswith('win32'):
    openssl_includes = [];
  else:
    openssl_includes = [normpath(join(cwd, '../openssl/include'))];

  libssl = conf.check_cc(lib="rand",
          header_name='openssl/rand.h',
          function_name='SSL',
          includes=openssl_includes,
          libpath=openssl_libpath,
          uselib_store='OPENSSL')


  #if conf.check(lib='ssl', libpath=['/usr/lib'], uselib_store='LIBSSL'):
    #conf.fatal("Cannot find openssl libraries (used for randomness).")

def build(bld):
  bcryptnode = bld.new_task_gen("cxx", "shlib", "node_addon")
  bcryptnode.target = "bcrypt_lib"
  bcryptnode.source = """
    src/blowfish.cc
    src/bcrypt.cc
    src/bcrypt_node.cc
  """

  #bcryptnode.includes = """
    #/usr/includes/openssl/
  #"""
  bcryptnode.uselib = 'OPENSSL'

def test(t):
  Utils.exec_command('nodeunit test')

def shutdown():
  t = 'bcrypt_lib.node'
  if Options.commands['clean']:
    if exists(t): unlink(t)
  if Options.commands['build']:
    if exists('build/default/' + t) and not exists(t):
      symlink('build/default/' + t, t)
