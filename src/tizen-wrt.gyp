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
        'power/power.gyp:*',
        'messageport/messageport.gyp:*',
        'bookmark/bookmark.gyp:*',
        'archive/archive.gyp:*',
        'exif/exif.gyp:*',
      ],
      'conditions': [
        [ 'extension_host_os == "mobile"', {
          'dependencies': [
            'callhistory/callhistory.gyp:*',
            'contact/contact.gyp:*',
            'calendar/calendar.gyp:*',
            'datasync/datasync.gyp:*',
            'messaging/messaging.gyp:*',
            'nfc/nfc.gyp:*'
          ],
        },
       ],
     ],
    },
  ],
}
