{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_keymanager',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'keymanager_api.js',
        'async_file_reader.cc',
        'async_file_reader.h',
        'keymanager_extension.cc',
        'keymanager_extension.h',
        'keymanager_instance.cc',
        'keymanager_instance.h',
        'keymanager_observers.h',
        'keymanager_observers.cc',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'key-manager',
              'libpcrecpp',
            ]
          },
        }],
      ],
    },
  ],
}