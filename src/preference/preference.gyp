{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_preference',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'preference_api.js',
        'preference_extension.cc',
        'preference_extension.h',
        'preference_instance.cc',
        'preference_instance.h',
        'preference_manager.cc',
        'preference_manager.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-application',
            ]
          },
        }],
      ],
    },
  ],
}
