{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_networkbearerselection',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'variables': {
        'packages': [
          'icu-i18n',
          'capi-network-connection',
        ],
      },
      'sources': [
        'networkbearerselection_api.js',
        'networkbearerselection_extension.cc',
        'networkbearerselection_extension.h',
        'networkbearerselection_instance.cc',
        'networkbearerselection_instance.h',
        'networkbearerselection_manager.cc',
        'networkbearerselection_manager.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { 'packages': ['vconf'] },
        }],
      ],
    },
  ],
}
