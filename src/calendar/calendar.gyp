{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_calendar',
      'type': 'loadable_module',
      'sources': [
        'calendar_api.js',
        'calendar_extension.cc',
        'calendar_extension.h',
        'calendar_instance.cc',
        'calendar_instance.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'glib-2.0',
              'calendar-service2',
              'vconf',
            ]
          },
        }],
      ],
    },
  ],
}
