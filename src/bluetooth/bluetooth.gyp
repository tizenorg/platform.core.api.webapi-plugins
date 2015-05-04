{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_bluetooth',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'bluetooth_api.js',
        'bluetooth_adapter.cc',
        'bluetooth_adapter.h',
        'bluetooth_class.cc',
        'bluetooth_class.h',
        'bluetooth_device.cc',
        'bluetooth_device.h',
        'bluetooth_extension.cc',
        'bluetooth_extension.h',
        'bluetooth_health_application.cc',
        'bluetooth_health_application.h',
        'bluetooth_health_channel.cc',
        'bluetooth_health_channel.h',
        'bluetooth_health_profile_handler.cc',
        'bluetooth_health_profile_handler.h',
        'bluetooth_instance.cc',
        'bluetooth_instance.h',
        'bluetooth_le_adapter.cc',
        'bluetooth_le_adapter.h',
        'bluetooth_service_handler.cc',
        'bluetooth_service_handler.h',
        'bluetooth_socket.cc',
        'bluetooth_socket.h',
        'bluetooth_util.cc',
        'bluetooth_util.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
         'variables': {
            'packages': [
              'capi-network-bluetooth',
              'capi-system-info',
              'libpcrecpp',
            ]
          },
        }],
      ],
    },
  ],
}
