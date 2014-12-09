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
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { },
        }],
      ],
    },
  ],
}
