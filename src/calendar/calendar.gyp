{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_calendar',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'calendar_api.js',
        'calendar_extension.cc',
        'calendar_extension.h',
        'calendar_instance.cc',
        'calendar_instance.h',
        'calendar.cc',
        'calendar.h',
        'calendar_item.cc',
        'calendar_item.h',
        'calendar_manager.cc',
        'calendar_manager.h',
        'calendar_privilege.h',
        'calendar_record.cc',
        'calendar_record.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'calendar-service2',
            ]
          },
        }],
      ],
    },
  ],
}
