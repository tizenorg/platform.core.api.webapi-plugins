{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_humanactivitymonitor',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'humanactivitymonitor_api.js',
        'humanactivitymonitor_extension.cc',
        'humanactivitymonitor_extension.h',
        'humanactivitymonitor_instance.cc',
        'humanactivitymonitor_instance.h',
        'humanactivitymonitor_manager.cc',
        'humanactivitymonitor_manager.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'motion',
              'capi-system-sensor',
              'capi-location-manager',
            ]
          },
        }],
      ],
    },
  ],
}
