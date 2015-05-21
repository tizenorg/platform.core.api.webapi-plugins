{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_sensor',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'sensor_api.js',
        'sensor_extension.cc',
        'sensor_extension.h',
        'sensor_instance.cc',
        'sensor_instance.h',
        'sensor_service.cc',
        'sensor_service.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-system-sensor',
            ]
          },
        }],
      ],
    },
  ],
}