{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_mediakey',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'mediakey_api.js',
        'mediakey_extension.cc',
        'mediakey_extension.h',
        'mediakey_instance.cc',
        'mediakey_instance.h',
        'mediakey_manager.cc',
        'mediakey_manager.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
         'variables': {
            'packages': [
              'capi-system-media-key',
            ]
          },
        }],
      ],
    },
  ],
}
