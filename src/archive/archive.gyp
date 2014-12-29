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
        #'archive_callback_data.cc',
        'archive_callback_data.h',
        'archive_extension.cc',
        'archive_extension.h',
        'archive_file.cc',
        'archive_file.h',
        'archive_file_entry.cc',
        'archive_file_entry.h',
        'archive_instance.cc',
        'archive_instance.h',
        'archive_manager.cc',
        'archive_manager.h',
        'archive_utils.cc',
        'archive_utils.h',
        'filesystem_file.cc'
        'filesystem_file.h',
        'defs.h',
        'un_zip.cc',
        'un_zip.h',
        'un_zip_extract_request.cc',
        'un_zip_extract_request.h',
        'zip_add_request.cc',
        'zip_add_request.h',
        'zip.cc',
        'zip.h'
      ],
      'includes': [
        '../common/pkg-config.gypi'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'minizip',
              'zlib'
            ]
          },
        }],
      ],
    },
  ],
}
