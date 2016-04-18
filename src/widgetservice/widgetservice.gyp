{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_widgetservice',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'widgetservice_api.js',
        'widgetservice_extension.cc',
        'widgetservice_extension.h',
        'widgetservice_instance.cc',
        'widgetservice_instance.h',
        'widgetservice_utils.cc',
        'widgetservice_utils.h',
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
