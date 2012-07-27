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
        '../src/bcrypt.cc',
		'../src/bcrypt_node.cc',
		'../src/blowfish.cc',
		'../src/node_blf.h',
      ],

	  'target_name': '<(module_name)',
      'type': '<(library)',
	  'product_name':'<(module_name)',
	  'product_extension':'node',
	  'product_dir':'../build/default',
	  #remove the default lib prefix on each library
      'product_prefix':'',
	  
      'defines': [
        'ARCH="<(target_arch)"',
        'PLATFORM="<(OS)"',
		'_LARGEFILE_SOURCE',
		'_FILE_OFFSET_BITS=64',
      ],
	  
      'include_dirs': [
        '<(NODE_ROOT)/src',
        '<(NODE_ROOT)/deps/v8/include',
        '<(NODE_ROOT)/deps/uv/include',
		'<(NODE_ROOT)/deps/openssl/openssl/include',
		'<(NODE_ROOT)/deps/openssl/config/k8/',
      ],

      'conditions': [
        [ 'OS=="win"', {
          'defines': [
            'uint=unsigned int',
            # we need to use node's preferred "win32" rather than gyp's preferred "win"
            'PLATFORM="win32"',
          ],
          'libraries': [ 
			'-l<(NODE_ROOT)/<(node_lib_folder)/node.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/http_parser.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/openssl.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/uv.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/v8_base.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/v8_nosnapshot.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/v8_snapshot.lib',
			'-l<(NODE_ROOT)/<(node_lib_folder)/lib/zlib.lib',
		  ],
		  'msvs_configuration_attributes': {
              'OutputDirectory': '..\\build\\default',
			  'IntermediateDirectory': '..\\build\\obj',
		  },
		  'msvs-settings': {
		    'VCLinkerTool': {
				'SubSystem': 3, # /subsystem:dll
		      },
		   },
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

