{
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
      ]
    }
  ]
}
