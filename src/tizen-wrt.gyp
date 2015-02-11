{
  'includes':[
    'common/common.gypi',
  ],

  'targets': [
    {
      'target_name': 'build_all_tizen_extensions',
      'type': 'none',
      'dependencies': [
        'tizen/tizen.gyp:*',
        'application/application.gyp:*',
        'package/package.gyp:*',
        'time/time.gyp:*',
        'utils/utils.gyp:*',
        'messageport/messageport.gyp:*',
        'archive/archive.gyp:*',
        'exif/exif.gyp:*',
        'websetting/websetting.gyp:*',
        'systemsetting/systemsetting.gyp:*',
      ],
      'conditions': [
        [
          'extension_host_os == "mobile"', {
            'dependencies': [
              'account/account.gyp:*',
              'badge/badge.gyp:*',
              'bluetooth/bluetooth.gyp:*',
              'callhistory/callhistory.gyp:*',
              'contact/contact.gyp:*',
              'calendar/calendar.gyp:*',
              'datacontrol/datacontrol.gyp:*',
              'datasync/datasync.gyp:*',
              'messaging/messaging.gyp:*',
              'networkbearerselection/networkbearerselection.gyp:*',
              'nfc/nfc.gyp:*',
              'power/power.gyp:*',
              'bookmark/bookmark.gyp:*',
              'systeminfo/systeminfo.gyp:*',
              #'radio/radio.gyp:*',
              'secureelement/secureelement.gyp:*',
              'sound/sound.gyp:*',
              'notification/notification.gyp:*',
              'sensor/sensor.gyp:*',
              'push/push.gyp:*',
            ],
          },
        ], # end mobile
        [
          'extension_host_os == "tv"', {
            'dependencies': [
              'tvdisplay/tvdisplay.gyp:*',
              'tvaudio/tvaudio.gyp:*',
              'tvchannel/tvchannel.gyp:*',
              'tvinputdevice/tvinputdevice.gyp:*',
              'tvwindow/tvwindow.gyp:*',
            ],
          },
        ], # end tv
      ], # end conditions
    },
  ], # end targets
}
