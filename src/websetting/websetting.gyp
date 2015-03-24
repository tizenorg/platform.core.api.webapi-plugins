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
        'websetting.cc',
        'websetting.h',
        'websetting_api.js',
        'websetting_extension.cc',
        'websetting_extension.h',
        'websetting_extension_utils.h',
        'websetting_instance.cc',
        'websetting_instance.h',
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
