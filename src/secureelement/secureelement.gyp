{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_secureelement',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'secureelement_api.js',
        'secureelement_extension.cc',
        'secureelement_extension.h',
        'secureelement_instance.cc',
        'secureelement_instance.h',
        'secureelement_seservice.cc',
        'secureelement_seservice.h',
        'secureelement_reader.cc',
        'secureelement_reader.h',
        'secureelement_session.cc',
        'secureelement_session.h',
        'secureelement_channel.cc',
        'secureelement_channel.h',
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
