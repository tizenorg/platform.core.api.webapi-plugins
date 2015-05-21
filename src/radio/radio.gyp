{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_fmradio',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'radio_api.js',
        'radio_extension.cc',
        'radio_extension.h',
        'radio_manager.cc',
        'radio_manager.h',
        'radio_instance.cc',
        'radio_instance.h'
      ],
       'includes': [
        '../common/pkg-config.gypi',
        ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
                'vconf',
                'capi-media-radio',
                'capi-system-runtime-info',
            ]
          },
        }],
      ],
    },
  ],
}
