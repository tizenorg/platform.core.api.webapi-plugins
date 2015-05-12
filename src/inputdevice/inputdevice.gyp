{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_inputdevice',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'inputdevice_api.js',
        'inputdevice_extension.cc',
        'inputdevice_extension.h',
        'inputdevice_instance.cc',
        'inputdevice_instance.h',
        'inputdevice_key.cc',
        'inputdevice_key.h',
        'inputdevice_manager.cc',
        'inputdevice_manager.h'
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
