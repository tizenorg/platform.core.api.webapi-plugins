{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_feedback',
      'type': 'loadable_module',
      'dependencies': [
        '../common/common.gyp:tizen_common',
      ],
      'sources': [
        'feedback_api.js',
        'feedback_extension.cc',
        'feedback_extension.h',
        'feedback_instance.cc',
        'feedback_instance.h',
        'feedback_manager.cc',
        'feedback_manager.h'
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
            'packages': [
              'feedback'
            ]
          },
        }],
      ],
    },
  ],
}
