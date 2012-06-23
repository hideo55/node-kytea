{
  'targets': [
    {
      'target_name': 'kytea',
      'sources': ['src/node_kytea.cc'],
      'cflags': ['-fexceptions'],
      'cflags_cc': ['-fexceptions'],
      'cflags!': ['-fno-exceptions'],
      'cflags_cc!': ['-fno-exception'],
      'libraries': ['-lkytea'],
      'conditions': [
        ['OS=="mac"', {
            'xcode_settings': {
              'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
              'GCC_ENABLE_CPP_RTTI': 'YES'
            }
          }
        ]
      ]
    }
  ]
}
