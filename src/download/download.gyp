{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_download',
      'type': 'loadable_module',
      'sources': [
        'download_api.js',
        'download_extension.cc',
        'download_extension.h',
        'download_instance.cc',
        'download_instance.h',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
           	'capi-web-url-download',
		'capi-system-info'
            ]
          },
        }],
      ],
    },
  ],
}
