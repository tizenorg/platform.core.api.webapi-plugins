{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_exif',
      'type': 'loadable_module',
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'exif_api.js',
        'exif_extension.cc',
        'exif_extension.h',
        'exif_instance.cc',
        'exif_instance.h',

        'exif_information.cc',
        'exif_information.h',
        'exif_util.cc',
        'exif_util.h',
        'exif_tag_saver.cc',
        'exif_tag_saver.h',
        'jpeg_file.cc',
        'jpeg_file.h',
        'rational.cc',
        'rational.h',
        'exif_gps_location.cc',
        'exif_gps_location.h',
        'get_exif_info.cc',
        'get_exif_info.h',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': {
              'packages': [
                'libexif',
              ]
            },
        }],
      ],
    },
  ],
}
