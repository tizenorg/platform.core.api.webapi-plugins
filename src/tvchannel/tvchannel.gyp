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
      'conditions': [
       ['tizen == 1', {
        'variables': {
            'packages': [
             'tvs-api'
            ],
        },
       },
      ],
      ],
      'sources': [
        'tvchannel_api.js',
        'tvchannel_error.h',
        'tvchannel_extension.cc',
        'tvchannel_extension.h',
        'tvchannel_instance.cc',
        'tvchannel_instance.h',
        'channel_info.cc',
        'channel_info.h',
        'program_info.cc',
        'program_info.h',
        'tvchannel_manager.h',
        'tvchannel_manager.cc',
        'types.h',
        'types.cc'
      ],
    },
  ],
}
