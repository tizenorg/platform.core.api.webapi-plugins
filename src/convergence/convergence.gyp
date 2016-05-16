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
        'conv_lib_json.cpp',
        'conv_lib_json.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'libd2d-conv-manager',
              'capi-appfw-package-manager',
              'json-glib-1.0',
            ]
          },
        }],
      ],
    },
  ],
}
