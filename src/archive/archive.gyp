{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_archive',
      'type': 'loadable_module',
      'sources': [
        'archive_api.js',
        'archive_extension.cc',
        'archive_extension.h',
        'archive_instance.cc',
        'archive_instance.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'minizip'
            ]
          },
        }],
      ],
    },
  ],
}
