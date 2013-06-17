{
    "targets": [
    {
      "target_name": "xblab",
      "sources": [  "../src/xblab.client.cc", "../src/util.cc",
      "../src/protobuf/xblab.pb.cc", "../src/crypto.cc",
      "../src/participant.cc" ],
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
            'GCC_ENABLE_CPP_RTTI': 'YES'
          }
        }]        
      ]
    }
  ]
}
