{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_content',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'content_api.js',
        'content_extension.cc',
        'content_extension.h',
        'content_filter.cc',
        'content_filter.h',
        'content_instance.cc',
        'content_instance.h',
        'content_manager.h',
        'content_manager.cc',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-content-media-content',
              'capi-media-metadata-extractor',
				      'capi-base-common',
              'dlog',
            ]
          },
        }],
      ],
    },
  ],
}
