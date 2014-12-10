{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_power',
      'type': 'loadable_module',
      'sources': [
        'power_api.js',
        'power_extension.cc',
        'power_extension.h',
        'power_instance.cc',
        'power_instance.h',
        'power_manager.cc',
        'power_manager.h',
        'power_platform_proxy.cc',
        'power_platform_proxy.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'glib-2.0',
              'capi-system-device',
              'capi-system-power',
              'vconf',
            ]
          },
        }],
      ],
    },
  ],
}
