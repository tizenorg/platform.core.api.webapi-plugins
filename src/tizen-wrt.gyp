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
        'time/time.gyp:*',
        'utils/utils.gyp:*',
        'messageport/messageport.gyp:*',
        'archive/archive.gyp:*',
        'exif/exif.gyp:*',
      ],
      'conditions': [
        [
          'extension_host_os == "mobile"', {
            'dependencies': [
              'callhistory/callhistory.gyp:*',
              'contact/contact.gyp:*',
              'calendar/calendar.gyp:*',
              'datasync/datasync.gyp:*',
              'messaging/messaging.gyp:*',
              'nfc/nfc.gyp:*',
              'power/power.gyp:*',
              'bookmark/bookmark.gyp:*',
              'systeminfo/systeminfo.gyp:*',
              #'radio/radio.gyp:*',
            ],
          },
        ], # end mobile
        [
          'extension_host_os == "tv"', {
            'dependencies': [
              # 'tvdisplay/tvdisplay.gyp:*',
              # 'tvchannel/tvchannel.gyp:*',
              # 'tvaudio/tvaudio.gyp:*',
            ],
          },
        ], # end tv
      ], # end conditions
    },
  ], # end targets
}
