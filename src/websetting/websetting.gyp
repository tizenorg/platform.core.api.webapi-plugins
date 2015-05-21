{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_websetting',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'websetting_api.js',
        'websetting_extension.cc',
        'websetting_extension.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'variables': {
        'packages': [
          'glib-2.0',
        ]
      },
    },
  ],
}
