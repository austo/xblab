{
    "targets": [
    {
      "target_name": "xblab",
      "sources": [  "../src/xblab.server.cc", "../src/util.cc",
      "../src/db.cc", "../src/nodeUtil.cc", "../src/crypto.cc", 
      "../src/manager.cc", "../src/protobuf/xblab.pb.cc" ],
      'conditions': [
          ['OS!="win"', {
            'cflags': [ '-Wall', '-fexceptions', '-g' ],
            'cflags_cc!': [ '-fno-exceptions', '-fno-rtti' ],
            'link_settings': {
              'libraries': [ '-lbotan-1.10', '-lprotobuf', '-lpqxx', '-lpq' ]
            },
            'include_dirs': [ '/usr/local/include/pqxx', '/usr/local/include/botan-1.10', '../src/' ]
          }
        ],
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'GCC_ENABLE_CPP_RTTI': 'YES'
          },
          'ldflags': [ '-Wl', '-no_implicit_dylibs'],
        }]        
      ]
    }
  ]
}
