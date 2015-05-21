{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'tizen.h',
        'tizen_api.js',
        'tizen_extension.cc',
        'tizen_extension.h',
      ],
    },
  ],
}
