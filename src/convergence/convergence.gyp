{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_convergence',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'convergence_api.js',
        'convergence_extension.cc',
        'convergence_extension.h',
        'convergence_instance.cc',
        'convergence_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-package-manager',
            ]
          },
        }],
      ],
    },
  ],
}
