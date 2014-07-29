{
  'targets': [
    {
      'target_name': 'bcrypt_lib',
      'sources': [
        'src/blowfish.cc',
        'src/bcrypt.cc',
        'src/bcrypt_node.cc'
      ],
      'include_dirs' : [
          "<!(node -e \"require('nan')\")"
      ],
      'conditions': [
        [ 'OS=="win"', {
          'defines': [
            'uint=unsigned int',
          ],
        }],
      ],
    }
  ]
}
