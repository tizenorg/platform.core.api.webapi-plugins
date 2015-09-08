{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_systeminfo',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'systeminfo_api.js',
        'systeminfo_extension.cc',
        'systeminfo_extension.h',
        'systeminfo_instance.cc',
        'systeminfo_instance.h',
        'systeminfo-utils.cpp',
        'systeminfo-utils.h',
        'systeminfo_manager.cc',
        'systeminfo_manager.h',
        'systeminfo_device_capability.cc',
        'systeminfo_device_capability.h',
        'systeminfo_properties_manager.cc',
        'systeminfo_properties_manager.h',
        'systeminfo_sim_details_manager.cc',
        'systeminfo_sim_details_manager.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
            'ecore',
            'vconf',
            'glib-2.0',
            'capi-system-info',
            'capi-network-connection',
            'capi-system-device',
            'capi-system-system-settings',
            'capi-network-wifi',
            'tapi',
            'sensor',
            ]
          },
        }],
        ['tizen_is_emulator == 1', {
          'defines': ['TIZEN_IS_EMULATOR'],
        }],
      ],
    },
  ],
}
