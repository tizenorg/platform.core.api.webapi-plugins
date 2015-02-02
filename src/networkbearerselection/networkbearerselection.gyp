{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_networkbearerselection',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'icu-i18n',
        ],
      },
      'sources': [
        'networkbearerselection_api.js',
        'networkbearerselection_extension.cc',
        'networkbearerselection_extension.h',
        'networkbearerselection_instance.cc',
        'networkbearerselection_instance.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { 'packages': ['vconf'] },
        }],
      ],
    },
  ],
}
