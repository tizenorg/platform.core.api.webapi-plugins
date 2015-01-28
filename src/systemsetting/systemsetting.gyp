{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_systemsetting',
      'type': 'loadable_module',
      'sources': [
        'systemsetting_api.js',
        'systemsetting_extension.cc',
        'systemsetting_extension.h',
        'systemsetting_instance.cc',
        'systemsetting_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-system-system-settings',
              'vconf',
            ]
          },
        }],
      ],
    },
  ],
}
