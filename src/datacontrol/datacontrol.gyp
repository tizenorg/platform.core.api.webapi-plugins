{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_datacontrol',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'datacontrol_api.js',
        'datacontrol_extension.cc',
        'datacontrol_extension.h',
        'datacontrol_instance.cc',
        'datacontrol_instance.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-data-control',
              'capi-base-common'
            ]
          },
        }],
      ],
    },
  ],
}
