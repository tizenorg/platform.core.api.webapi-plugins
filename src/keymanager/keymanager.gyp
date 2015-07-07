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
        'keymanager_extension.cc',
        'keymanager_extension.h',
        'keymanager_instance.cc',
        'keymanager_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'key-manager',
              'pkgmgr-info',
            ]
          },
        }],
      ],
    },
  ],
}
