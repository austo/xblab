{
    "targets": [
    {
      "target_name": "xblab",
      "sources": [   "../src/client/client.cc",
      "../src/client/memberBaton.cc",
      "../src/client/batonUtil.cc",
      "../src/client/binding/nodeUtil.cc",
      "../src/protobuf/xblab.pb.cc",
      "../src/common/crypto.cc",
      "../src/client/binding/xbClient.cc" ],
      'defines': [ 'XBLAB_CLIENT', 'DEBUG' ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall', '-fexceptions', '-g' ],
            'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
            'link_settings': {
              'libraries': [ '-lbotan-1.10', '-lprotobuf-lite' ]
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
