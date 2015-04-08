{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_mediacontroller',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'mediacontroller_api.js',
        'mediacontroller_extension.cc',
        'mediacontroller_extension.h',
        'mediacontroller_instance.cc',
        'mediacontroller_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-media-controller',
            ]
          },
        }],
      ],
    },
  ],
}
