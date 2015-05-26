{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_common',
      'type': 'loadable_module',
      'sources': [
        'converter.cc',
        'converter.h',
        'current_application.cc',
        'current_application.h',
        'extension.cc',
        'extension.h',
        'filter-utils.cc',
        'filter-utils.h',
        'picojson.h',
        'utils.h',
        'logger.cc',
        'logger.h',
        'platform_exception.cc',
        'platform_exception.h',
        'dbus_operation.cc',
        'dbus_operation.h',
        'XW_Extension.h',
        'XW_Extension_EntryPoints.h',
        'XW_Extension_Permissions.h',
        'XW_Extension_Runtime.h',
        'XW_Extension_SyncMessage.h',
        'scope_exit.h',
        'task-queue.cpp',
        'task-queue.h',
        'callback_user_data.cc',
        'callback_user_data.h',
        'optional.h',
        'platform_result.cc',
        'platform_result.h',
        'assert.h',
        'virtual_fs.cc',
        'virtual_fs.h',
      ],
      'cflags': [
        '-fvisibility=default',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'capi-appfw-app-manager',
              'capi-appfw-package-manager',
              'storage',
            ]
          },
          'conditions': [
            ['privilege_engine == "DB"', {
              'defines': ['PRIVILEGE_USE_DB'],
              'variables': {
                'packages': [
                  'sqlite3',
                ],
              },
            }],
            ['privilege_engine == "ACE"', {
              'defines': ['PRIVILEGE_USE_ACE'],
              'variables': {
                'packages': [
                  'capi-security-privilege-manager',
                ],
              },
            }],
            ['privilege_engine == "CYNARA"', {
              'defines': ['PRIVILEGE_USE_CYNARA'],
              'variables': {
                'packages': [
                  'cynara-client',
                  'libsmack',
                ],
              },
            }],
          ],
        }],
      ],
      'direct_dependent_settings': {
        'libraries' : [
          '-ltizen_common',
        ],
        'variables': {
          'packages': [
            'storage',
          ],
        },
      },
    },
  ],
}
