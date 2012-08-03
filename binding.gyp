{
  'variables': {
    # The default install location of OpenSSL-Win32
    'openssl_Root': 'C:/OpenSSL-Win32'
  },
  'targets': [
    {
      'target_name': 'bcrypt_lib',
      'include_dirs': [
        '<(node_root_dir)/deps/openssl/openssl/include'
      ],
      'sources': [
        'src/blowfish.cc',
        'src/bcrypt.cc',
        'src/bcrypt_node.cc'
      ],
      'conditions': [
        [ 'OS=="win"', {
          'defines': [
            'uint=unsigned int',
            # we need to use node's preferred "win32" rather than gyp's preferred "win"
            'PLATFORM="win32"',
          ],
          'libraries': [ 
            '-l<(openssl_Root)/lib/libeay32.lib',
          ],
          'include_dirs': [
            '<(openssl_Root)/include',
          ],
        }],
      ],
    }
  ]
}
