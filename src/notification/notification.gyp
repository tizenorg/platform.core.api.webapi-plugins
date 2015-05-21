{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_notification',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'notification_api.js',
        'notification_extension.cc',
        'notification_extension.h',
        'notification_instance.cc',
        'notification_instance.h',
        'notification_manager.h',
        'notification_manager.cc',
        'status_notification.cc',
        'status_notification.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'notification',
              'capi-system-device',
            ]
          },
        }],
      ],
    },
  ],
}
