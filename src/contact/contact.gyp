{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_contact',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'contact_api.js',
        'contact_extension.cc',
        'contact_extension.h',
        'contact_instance.cc',
        'contact_instance.h',
        'addressbook.cc',
        'addressbook.h',
        'contact_manager.cc',
        'contact_manager.h',
        'contact_util.cc',
        'contact_util.h',
        'person.cc',
        'person.h',
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'contacts-service2',
            ]
          },
        }],
      ],
    },
  ],
}
