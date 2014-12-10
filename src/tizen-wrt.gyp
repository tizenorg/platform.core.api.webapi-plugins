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
        #'calendar/calendar.gyp:*',
        'bookmark/bookmark.gyp:*',
        #'datasync/datasync.gyp:*',
      ],
    },
  ],
}
