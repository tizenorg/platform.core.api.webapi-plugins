{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_iotcon',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'iotcon_api.js',
        'iotcon_extension.cc',
        'iotcon_extension.h',
        'iotcon_instance.cc',
        'iotcon_instance.h',
        'iotcon_server_manager.cc',
        'iotcon_server_manager.h',
        'iotcon_utils.cc',
        'iotcon_utils.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'iotcon'
            ]
          },
        }],
      ],
    },
  ],
}
