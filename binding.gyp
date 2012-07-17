{
  'variables': {
    #Specify the module name here
    'module_name': 'bcrypt_lib',
    #These are required variables to make a proper node module build
    'library': 'shared_library',
    'target_arch': 'ia32', 
  },
  'targets': [
    {
      'sources': [
        'src/bcrypt.cc',
        'src/bcrypt_node.cc',
        'src/blowfish.cc',
        'src/node_blf.h',
      ],

      'target_name': '<(module_name)',
      'type': '<(library)',
      'product_name':'<(module_name)',
      'product_extension':'node',
      #remove the default lib prefix on each library
      'product_prefix':'',

      'defines': [
        'ARCH="<(target_arch)"',
        'PLATFORM="<(OS)"',
        '_LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
      ],

      'include_dirs': [
        '<(node_root_dir)/src',
        '<(node_root_dir)/deps/v8/include',
        '<(node_root_dir)/deps/uv/include',
        '<(node_root_dir)/deps/openssl/openssl/include',
        '<(node_root_dir)/deps/openssl/config/k8/',
      ],

      'conditions': [
        [ 'OS=="win"', {
          'defines': [
            'uint=unsigned int',
            # we need to use node's preferred "win32" rather than gyp's preferred "win"
            'PLATFORM="win32"',
          ],
          'libraries': [ 
            '-l<(node_root_dir)/Release/node.lib',
            '-l<(module_root_dir)/deps/openssl.lib',
          ],
        }],
        [ 'OS=="mac"', {
           #MAC Users don't forget to comment out all line in node\tools\gyp\pylib\gyp\generator\make.py that contain append('-arch i386') (2 instances)
           'libraries': [ #this is a hack to specify this linker option in make
              '-undefined dynamic_lookup',
           ],
        }],
        [ 'OS=="linux"', {

        }]
      ],
    },
  ] # end targets
}
