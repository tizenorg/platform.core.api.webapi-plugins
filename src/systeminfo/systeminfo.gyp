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
            'tapi',
            'sensor',
            'libtzplatform-config',
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
