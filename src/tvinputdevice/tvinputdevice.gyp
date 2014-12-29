{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_tvinputdevice',
      'type': 'loadable_module',
      'sources': [
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
