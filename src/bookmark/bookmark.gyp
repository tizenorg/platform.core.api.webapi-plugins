{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_bookmark',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'bookmark_api.js',
        'bookmark_extension.cc',
        'bookmark_extension.h',
        'bookmark_instance.cc',
        'bookmark_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'bookmark-adaptor',
            ]
          },
        }],
      ],
    },
  ],
}
