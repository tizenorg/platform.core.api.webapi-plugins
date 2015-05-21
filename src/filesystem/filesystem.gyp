{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_filesystem',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'icu-i18n',
          'storage',
        ],
      },
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'filesystem_api.js',
        'filesystem_extension.cc',
        'filesystem_extension.h',
        'filesystem_file.cc',
        'filesystem_file.h',
        'filesystem_instance.cc',
        'filesystem_instance.h',
        'filesystem_manager.cc',
        'filesystem_manager.h',
        'filesystem_stat.cc',
        'filesystem_stat.h',
        'filesystem_utils.cc',
        'filesystem_utils.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { 'packages': [
              'vconf',
              'capi-appfw-app-manager',
              'capi-appfw-package-manager',
            ] },
        }],
      ],
    },
  ],
}
