{
  'includes':[
    'common/common.gypi',
  ],

  'targets': [
    {
      'target_name': 'build_all_tizen_extensions',
      'type': 'none',
      'dependencies': [
        'tool/tool.gyp:*',
        'common/common.gyp:*',
        'tizen/tizen.gyp:*',
        'utils/utils.gyp:*',
      ],
      'conditions': [
        [
          'tizen_feature_account_support==1', {
            'dependencies': [
              'account/account.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_alarm_support==1', {
            'dependencies': [
              'alarm/alarm.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_application_support==1', {
            'dependencies': [
              'application/application.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_archive_support==1', {
            'dependencies': [
              'archive/archive.gyp:*'
            ],
          },
        ],
        [
          'tizen_feature_badge_support==1', {
            'dependencies': [
              'badge/badge.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_bluetooth_support==1', {
            'dependencies': [
              'bluetooth/bluetooth.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_bookmark_support==1', {
            'dependencies': [
              'bookmark/bookmark.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_calendar_support==1', {
            'dependencies': [
              'calendar/calendar.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_callhistory_support==1', {
            'dependencies': [
              'callhistory/callhistory.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_contact_support==1', {
            'dependencies': [
              'contact/contact.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_content_support==1', {
            'dependencies': [
              'content/content.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_datacontrol_support==1', {
            'dependencies': [
              'datacontrol/datacontrol.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_datasync_support==1', {
            'dependencies': [
              'datasync/datasync.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_download_support==1', {
            'dependencies': [
              'download/download.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_exif_support==1', {
            'dependencies': [
              'exif/exif.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_filesystem_support==1', {
            'dependencies': [
              'filesystem/filesystem.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_fm_radio_support==1', {
            'dependencies': [
              'radio/radio.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_key_manager_support==1', {
            'dependencies': [
              'keymanager/keymanager.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_ham_support==1', {
            'dependencies': [
              'humanactivitymonitor/humanactivitymonitor.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_media_controller_support==1', {
            'dependencies': [
              'mediacontroller/mediacontroller.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_media_key_support==1', {
            'dependencies': [
              'mediakey/mediakey.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_message_port_support==1', {
            'dependencies': [
              'messageport/messageport.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_messaging_support==1', {
            'dependencies': [
              'messaging/messaging.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_nbs_support==1', {
            'dependencies': [
              'networkbearerselection/networkbearerselection.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_nfc_emulation_support==1', {
            'dependencies': [
              'nfc/nfc.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_nfc_support==1', {
            'dependencies': [
              'nfc/nfc.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_notification_support==1', {
            'dependencies': [
              'notification/notification.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_package_support==1', {
            'dependencies': [
              'package/package.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_power_support==1', {
            'dependencies': [
              'power/power.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_push_support==1', {
            'dependencies': [
              'push/push.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_se_support==1', {
            'dependencies': [
              'secureelement/secureelement.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_sensor_support==1', {
            'dependencies': [
              'sensor/sensor.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_sound_support==1', {
            'dependencies': [
              'sound/sound.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_system_info_support==1', {
            'dependencies': [
              'systeminfo/systeminfo.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_system_setting_support==1', {
            'dependencies': [
              'systemsetting/systemsetting.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_time_support==1', {
            'dependencies': [
              'time/time.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_tv_display_support==1', {
            'dependencies': [
              'tvdisplay/tvdisplay.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_tvaudio_support==1', {
            'dependencies': [
              'tvaudio/tvaudio.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_tvinputdevice_support==1', {
            'dependencies': [
              'tvinputdevice/tvinputdevice.gyp:*'
            ],
          },
        ],
        [
          'tizen_feature_inputdevice_support==1', {
            'dependencies': [
              'inputdevice/inputdevice.gyp:*'
            ],
          },
        ],
        [
          'tizen_feature_tvchannel_support==1', {
            'dependencies': [
              'tvchannel/tvchannel.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_tvwindow_support==1', {
            'dependencies': [
              'tvwindow/tvwindow.gyp:*',
            ],
          },
        ],
        [
          'tizen_feature_web_setting_support==1', {
            'dependencies': [
              'websetting/websetting.gyp:*',
            ],
          },
        ],
      ], # end conditions
    },
  ], # end targets
}
