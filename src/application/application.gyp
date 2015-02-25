{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_application',
      'type': 'loadable_module',
      'sources': [
        'application_api.js',
        'application_extension.cc',
        'application_extension.h',
        'application_instance.cc',
        'application_instance.h',
        'application.cc',
        'application.h',
        'application_information.cc',
        'application_information.h',
        'application_context.cc',
        'application_context.h',
        'application_metadata.cc',
        'application_metadata.h',
        'application_certificate.cc',
        'application_certificate.h',
        'application_control.cc',
        'application_control.h',
        'application_controldata.cc',
        'application_controldata.h',
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
