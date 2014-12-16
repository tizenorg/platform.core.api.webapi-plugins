
{
  'includes':[
    '../common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_messaging',
      'type': 'loadable_module',
      'variables': {
        'packages': [
            'msg-service',
            'email-service',
            'dbus-1',
            'dbus-glib-1',
            'capi-system-info',
            'tapi',
            'vconf'
        ],
      },
      'sources': [
        'messaging_api.js',
        'email_manager.cc',
        'email_manager.h',
        'messaging_instance.cc',
        'messaging_instance.h',
        'messaging_extension.cc',
        'messaging_extension.h',
        'messaging_manager.cc',
        'messaging_manager.h',
        'messaging_util.cc',
        'messaging_util.h',
        'message_service.cc',
        'message_service.h',
        'message_service_email.cc',
        'message_service_email.h',
        'message_storage.cc',
        'message_storage.h',
        'message_storage_email.cc',
        'message_storage_email.h',
        'message.cc',
        'message.h',
        'message_email.cc',
        'message_email.h',
        'message_attachment.cc',
        'message_attachment.h',
        'message_body.cc',
        'message_body.h'
      ],
      'includes': [
        '../common/pkg-config.gypi',
      ],
      'conditions': [
        ['tizen == 1', {
          'variables': {
          },
        }],
      ],
    },
  ],
}
