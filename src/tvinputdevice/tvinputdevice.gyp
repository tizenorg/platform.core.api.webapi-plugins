{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvinputdevice',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'tvinputdevice_api.js',
        'tvinputdevice_extension.cc',
        'tvinputdevice_extension.h',
        'tvinputdevice_instance.cc',
        'tvinputdevice_instance.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
            ]
          },
        }],
      ],
    },
  ],
}
