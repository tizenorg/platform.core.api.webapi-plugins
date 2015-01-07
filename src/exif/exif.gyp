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
      'sources': [
        'exif_api.js',
        'exif_extension.cc',
        'exif_extension.h',
        'exif_instance.cc',
        'exif_instance.h',

        'ExifInformation.cpp',
        'ExifInformation.h',
        'ExifUtil.cpp',
        'ExifUtil.h',
        'ExifTagSaver.cpp',
        'ExifTagSaver.h',
        'JpegFile.cpp',
        'JpegFile.h',
        'Rational.cpp',
        'Rational.h',
        'ExifGPSLocation.cpp',
        'ExifGPSLocation.h',
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
