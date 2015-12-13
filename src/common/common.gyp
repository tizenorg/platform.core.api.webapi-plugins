{
  'includes':[
    '../common/common.gypi',
  ],
  'variables': {
    'enable_common_debug_logs%': '0',
  },
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
        'XW_Extension.h',
        'XW_Extension_EntryPoints.h',
        'XW_Extension_Permissions.h',
        'XW_Extension_Runtime.h',
        'XW_Extension_SyncMessage.h',
        'scope_exit.h',
        'task-queue.cpp',
        'task-queue.h',
        'tools.cc',
        'tools.h',
        'optional.h',
        'platform_result.cc',
        'platform_result.h',
        'assert.h',
        'GDBus/connection.cpp',
        'GDBus/connection.h',
        'GDBus/proxy.cpp',
        'GDBus/proxy.h',
        'GDBus/gdbus_powerwrapper.cc',
        'GDBus/gdbus_powerwrapper.h',
        'GDBus/auto_gen_interface.c',
        'GDBus/auto_gen_interface.h',
        'filesystem/filesystem_storage_types.h',
        'filesystem/filesystem_storage.h',
        'filesystem/filesystem_storage.cc',
        'filesystem/filesystem_provider_storage.h',
        'filesystem/filesystem_provider_storage.cc',
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
                  'security-privilege-checker',
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
        ['extension_build_type == "Debug"', {
          'conditions': [
            ['enable_common_debug_logs == 0', {
              # remove TIZEN_DEBUG_ENABLE flag
              'defines!': ['TIZEN_DEBUG_ENABLE'],
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
