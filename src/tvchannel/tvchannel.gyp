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
        '../common/common.gyp:tizen_common',
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
        'types.cc',
        'tune_option.h',
        'tune_option.cc',
        'criteria_filter.h',
        'criteria_filter.cc'
      ],
    },
  ],
}
