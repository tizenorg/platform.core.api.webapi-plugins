{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_package',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'package_api.js',
        'package_extension.cc',
        'package_extension.h',
        'package_instance.cc',
        'package_instance.h',
        'package_info_provider.cc',
        'package_info_provider.h'
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
