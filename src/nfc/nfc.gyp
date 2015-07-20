{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_nfc',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'nfc_api.js',
        'nfc_extension.cc',
        'nfc_extension.h',
        'nfc_instance.cc',
        'nfc_instance.h',
        'nfc_adapter.cc',
        'nfc_adapter.h',
        'nfc_util.cc',
        'nfc_util.h',
        'nfc_message_utils.cc',
        'nfc_message_utils.h',
        'nfc_platform_callbacks.h',
        'aid_data.cc',
        'aid_data.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
                'capi-system-info',
                'capi-network-nfc',
                'capi-appfw-application'
            ]
          },
        }],
        ['tizen_feature_app_control_settings_support == 1', {
          'defines': ['APP_CONTROL_SETTINGS_SUPPORT'],
        }],
      ],
    },
  ],
}
