{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_notification',
      'type': 'loadable_module',
      'sources': [
        'notification_api.js',
        'notification_extension.cc',
        'notification_extension.h',
        'notification_instance.cc',
        'notification_instance.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'notification'
            ]
          },
        }],
      ],
    },
  ],
}
