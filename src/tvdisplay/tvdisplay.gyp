{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvdisplay',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'sources': [
        'tvdisplay_api.js',
        'tvdisplay_extension.cc',
        'tvdisplay_extension.h',
        'tvdisplay_instance.cc',
        'tvdisplay_instance.h',
      ],
      'conditions': [
        [
          'tizen == 1',
          {
            'variables': {
              'packages': [
                'capi-system-info',
                'vconf',
              ]
            },
          },
        ],
      ],
    },
  ],
}
