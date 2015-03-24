{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_messageport',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'messageport_api.js',
        'messageport_extension.cc',
        'messageport_extension.h',
        'messageport_instance.cc',
        'messageport_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'vconf',
              'capi-message-port',
              'dlog'
            ]
          },
        }],
      ],
    },
  ],
}
