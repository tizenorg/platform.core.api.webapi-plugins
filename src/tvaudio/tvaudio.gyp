{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvaudio',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'tvaudio_api.js',
        'tvaudio_extension.cc',
        'tvaudio_extension.h',
        'tvaudio_instance.cc',
        'tvaudio_instance.h',
        'tvaudio_manager.cc',
        'tvaudio_manager.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'glib-2.0',
              'libavoc',
              'capi-media-audio-io',
              'capi-media-sound-manager',
            ]
          },
        }],
      ],
    },
  ],
}
