{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_fmradio',
      'type': 'loadable_module',
      'sources': [
        'radio_api.js',
        'radio_extension.cc',
        'radio_extension.h',
        'radio_instance.cc',
        'radio_instance.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
                'vconf',
                'capi-media-radio',
            ]
          },
        }],
      ],
    },
  ],
}
