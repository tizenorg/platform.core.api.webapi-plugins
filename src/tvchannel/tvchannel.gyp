{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvchannel',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'tvs-api',
        ],
      },
      'dependencies': [
        '../tizen/tizen.gyp:tizen',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'sources': [
        'tvchannel_api.js',
        'tvchannel_error.h',
        'tvchannel_extension.cc',
        'tvchannel_extension.h',
        'tvchannel_instance.cc',
        'tvchannel_instance.h',
      ],
    },
  ],
}
