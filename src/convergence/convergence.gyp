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
        'convergence_manager.cc',
        'convergence_manager.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'd2d-conv-manager',
            ]
          },
        }],
      ],
    },
  ],
}
