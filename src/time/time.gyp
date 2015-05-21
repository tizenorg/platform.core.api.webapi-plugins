{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_time',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'variables': {
        'packages': [
          'icu-i18n',
        ],
      },
      'sources': [
        'time_api.js',
        'time_extension.cc',
        'time_extension.h',
        'time_instance.cc',
        'time_instance.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { 'packages': ['vconf'] },
        }],
      ],
    },
  ],
}
