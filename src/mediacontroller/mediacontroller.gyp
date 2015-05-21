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
        'mediacontroller_client.cc',
        'mediacontroller_client.h',
        'mediacontroller_extension.cc',
        'mediacontroller_extension.h',
        'mediacontroller_instance.cc',
        'mediacontroller_instance.h',
        'mediacontroller_server.cc',
        'mediacontroller_server.h',
        'mediacontroller_types.cc',
        'mediacontroller_types.h',
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
