{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_callhistory',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'callhistory_api.js',
        'callhistory.cc',
        'callhistory.h',
        'callhistory_extension.cc',
        'callhistory_extension.h',
        'callhistory_instance.cc',
        'callhistory_instance.h',
        'callhistory_utils.cc',
        'callhistory_utils.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'glib-2.0',
              'contacts-service2',
              'libpcrecpp',
              'tapi',
            ]
          },
        }],
      ],
    },
  ],
}