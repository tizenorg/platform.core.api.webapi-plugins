{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_push',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'push_api.js',
        'push_extension.cc',
        'push_extension.h',
        'push_instance.cc',
        'push_instance.h',
        'push_manager.cc',
        'push_manager.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'push',
              'capi-appfw-application',
              'libpcrecpp',
            ]
          },
        }],
      ],
    },
  ],
}
