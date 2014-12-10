
{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_messaging',
      'type': 'loadable_module',
      'sources': [
        'messaging_api.js',
        'messaging_instance.cc',
        'messaging_instance.h',
        'messaging_extension.cc',
        'messaging_extension.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
            ]
          },
        }],
      ],
    },
  ],
}
