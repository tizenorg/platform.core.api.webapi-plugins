
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
            'ecore',
            'tapi',
            'vconf',
            'db-util'
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
        'message_body.h',
        'message_callback_user_data.cc',
        'message_callback_user_data.h',
        'find_msg_callback_user_data.cc',
        'find_msg_callback_user_data.h',
        'messaging_database_manager.cc',
        'messaging_databese_manager.h',
        'DBus/Connection.cpp',
        'DBus/Connection.h',
        'DBus/EmailSignalProxy.cpp',
        'DBus/EmailSignalProxy.h',
        #'DBus/LoadAttachmentProxy.cpp',
        #'DBus/LoadAttachmentProxy.h',
        'DBus/LoadBodyProxy.cpp',
        'DBus/LoadBodyProxy.h',
        'DBus/MessageProxy.cpp',
        'DBus/MessageProxy.h',
        'DBus/Proxy.cpp',
        'DBus/Proxy.h',
        'DBus/SendProxy.cpp',
        'DBus/SendProxy.h',
        'DBus/SyncProxy.cpp',
        'DBus/SyncProxy.h',
        'MsgCommon/Any.cpp',
        'MsgCommon/Any.h',
        'MsgCommon/AbstractFilter.cpp',
        'MsgCommon/AbstractFilter.h',
        'MsgCommon/SortMode.cpp',
        'MsgCommon/SortMode.h',
        'MsgCommon/AttributeFilter.cpp',
        'MsgCommon/AttributeFilter.h',
        'MsgCommon/AttributeRangeFilter.cpp',
        'MsgCommon/AttributeRangeFilter.h',
        'MsgCommon/CompositeFilter.cpp',
        'MsgCommon/CompositeFilter.h',
        'MsgCommon/FilterIterator.cpp',
        'MsgCommon/FilterIterator.h',
        'message_callback_user_data.cc',
        'message_callback_user_data.h',
        'messages_callback_user_data.cc',
        'messages_callback_user_data.h',
        'change_listener_container.cc',
        'change_listener_container.h',
        'messages_change_callback.cc',
        'messages_change_callback.h',
        'message_folder.cc',
        'message_folder.h'
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
