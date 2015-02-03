{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_sound',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'icu-i18n',
        ],
      },
      'sources': [
        'sound_api.js',
        'sound_extension.cc',
        'sound_extension.h',
        'sound_instance.cc',
        'sound_instance.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': {
              'packages': [
                'vconf',
                'capi-media-sound-manager',
              ]
            },
        }],
      ],
    },
  ],
}
