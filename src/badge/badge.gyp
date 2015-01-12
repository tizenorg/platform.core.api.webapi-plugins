{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_badge',
      'type': 'loadable_module',
      'sources': [
        'badge_api.js',
        'badge_extension.cc',
        'badge_extension.h',
        'badge_instance.cc',
        'badge_instance.h',
        'badge_manager.cc',
        'badge_manager.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'aul',
              'badge',
              'capi-appfw-package-manager',
              'vconf',
            ]
          },
        }],
      ],
    },
  ],
}
