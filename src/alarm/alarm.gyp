{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_alarm',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'alarm_api.js',
        'alarm_extension.cc',
        'alarm_extension.h',
        'alarm_instance.cc',
        'alarm_instance.h',
        'alarm_manager.cc',
        'alarm_manager.h',
        'alarm_utils.cc',
        'alarm_utils.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-alarm',
            ]
          },
        }],
      ],
    },
  ],
}