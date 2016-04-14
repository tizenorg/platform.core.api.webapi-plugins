{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_widget',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'widget_api.js',
        'widget_extension.cc',
        'widget_extension.h',
        'widget_instance.cc',
        'widget_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'widget_service',
            ]
          },
        }],
      ],
    },
  ],
}
