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
          "<!@(node -p \"require('node-addon-api').include\")"
      ],
      'msvs_settings': {
        'VCCLCompilerTool': { 'ExceptionHandling': 1 },
      },
      "xcode_settings": {
        "CLANG_CXX_LIBRARY": "libc++",
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES'
      },
      'conditions': [
        [ 'OS=="win"', {
          'defines': [
            'uint=unsigned int',
          ],
        }],
      ],
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
