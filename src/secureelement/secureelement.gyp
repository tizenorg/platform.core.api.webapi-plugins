{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_secureelement',
      'type': 'loadable_module',
      'sources': [
        'secureelement_api.js',
        'secureelement_extension.cc',
        'secureelement_extension.h',
        'secureelement_instance.cc',
        'secureelement_instance.h',
        'secureelement_seservice.cc',
        'secureelement_seservice.h',
        'secureelement_reader.cc',
        'secureelement_reader.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        [ 'tizen == 1', {
            'variables': { 'packages': ['smartcard-service', 'smartcard-service-common'] },
        }],
      ],
    },
  ],
}
