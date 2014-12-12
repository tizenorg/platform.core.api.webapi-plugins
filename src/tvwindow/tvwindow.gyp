{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvwindow',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'tvs-api',
        ],
      },
      'dependencies': [
        '../tizen/tizen.gyp:tizen',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'sources': [
        'tvwindow_api.js',
        'tvwindow_extension.cc',
        'tvwindow_extension.h',
        'tvwindow_manager.cc',
        'tvwindow_manager.h',
        'tvwindow_instance.cc',
        'tvwindow_instance.h',
      ],
    },
  ],
}
