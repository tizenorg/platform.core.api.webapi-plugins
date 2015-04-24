{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_application',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'application_api.js',
        'application_extension.cc',
        'application_extension.h',
        'application_instance.cc',
        'application_instance.h',
        'application.cc',
        'application.h',
        'application_manager.cc',
        'application_manager.h',
        'application_utils.cc',
        'application_utils.h',
        'requested_application_control.cc',
        'requested_application_control.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
         'variables': {
            'packages': [
              'aul',
              'capi-appfw-app-manager',
              'capi-appfw-application',
              'capi-appfw-package-manager',
              'pkgmgr',
              'pkgmgr-info',
            ]
          },
        }],
      ],
    },
  ],
}
