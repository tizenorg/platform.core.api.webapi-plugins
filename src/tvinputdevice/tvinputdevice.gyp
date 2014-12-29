{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvinputdevice',
      'type': 'loadable_module',
      'sources': [
        'tvinputdevice_api.js',
        'tvinputdevice_extension.cc',
        'tvinputdevice_extension.h',
        'tvinputdevice_instance.cc',
        'tvinputdevice_instance.h',
        'tvinputdevice_key.cc',
        'tvinputdevice_key.h',
        'tvinputdevice_manager.cc',
        'tvinputdevice_manager.h'
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
