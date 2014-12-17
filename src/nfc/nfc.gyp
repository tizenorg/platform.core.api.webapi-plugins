{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_nfc',
      'type': 'loadable_module',
      'sources': [
        'nfc_api.js',
        'nfc_extension.cc',
        'nfc_extension.h',
        'nfc_instance.cc',
        'nfc_instance.h',
        'nfc_adapter.cc',
        'nfc_adapter.h',
        'nfc_util.cc',
        'nfc_util.h'
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
      ],
    },
  ],
}
