{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_datasync',
      'type': 'loadable_module',
      'variables': {
        'packages': [
          'sync-agent',
        ],
      },
      'dependencies': [
        '../tizen/tizen.gyp:tizen',
      ],
      'sources': [
        'datasync_api.js',
        'datasync_extension.cc',
        'datasync_extension.h',
        'datasync_instance.cc',
        'datasync_instance.h',
        'datasync_manager.cc',
        'datasync_manager.h',
      ],
    },
  ],
}
