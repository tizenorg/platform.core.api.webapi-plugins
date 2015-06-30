{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_utils',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'utils_api.js',
        'utils_extension.cc',
        'utils_extension.h',
        'utils_instance.cc',
        'utils_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-package-manager',
              'capi-appfw-app-manager',
              'pkgmgr-info',
              'pkgmgr'
            ]
          },
        }],
      ],
    },
  ],
}
