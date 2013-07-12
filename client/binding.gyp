{
    "targets": [
    {
      "target_name": "xblab",
      "sources": [  "../src/native/participantBaton.cc",
      "../src/native/participantUtil.cc",
      "../src/binding/nodeUtil.cc", "../src/protobuf/xblab.pb.cc",
      "../src/crypto.cc", "../src/binding/xbClient.cc" ],
      'defines': [ 'XBLAB_CLIENT' ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall', '-fexceptions', '-g' ],
            'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
            'link_settings': {
              'libraries': [ '-lbotan-1.10', '-lprotobuf' ]
            },
            'include_dirs': [ '/usr/local/include/botan-1.10', '../src/' ]
          }
        ],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES',
            'MACOSX_DEPLOYMENT_TARGET': '10.6'
          }          
        }]        
      ]
    }
  ]
}
