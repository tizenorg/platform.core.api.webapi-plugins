{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_account',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'account_api.js',
        'account_extension.cc',
        'account_extension.h',
        'account_instance.cc',
        'account_instance.h',
        'account_manager.cc',
        'account_manager.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-package-manager',
              'accounts-svc',
            ]
          },
        }],
      ],
    },
  ],
}
