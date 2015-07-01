{
  'variables': {
    'extension_host_os%': 'desktop',
    'tizen%': '0',
    'extension_build_type%': '<(extension_build_type)',
    'extension_build_type%': 'Debug',
    'display_type%': 'x11',
  },
  'target_defaults': {
    'conditions': [
      ['extension_host_os != "mobile"', {
        'sources/': [['exclude', '_mobile\\.cc$|mobile/']],
        'includes/': [['exclude', '_mobile\\.gypi$|mobile/']],
      }],
      ['extension_host_os != "wearable"', {
        'sources/': [['exclude', '_wearable\\.cc$|wearable/']],
        'includes/': [['exclude', '_wearable\\.gypi$|wearable/']],
      }],
      ['extension_host_os != "tv"', {
        'sources/': [['exclude', '_tv\\.cc$|tv/']],
        'includes/': [['exclude', '_tv\\.gypi$|tv/']],
      }],
      ['extension_host_os != "desktop"', {
        'sources/': [['exclude', '_desktop\\.cc$|desktop/']],
        'includes/': [['exclude', '_desktop\\.gypi$|desktop/']],
      }],
      ['tizen == 1', {
        'defines': ['TIZEN'],
        'variables': {
          'packages': [
            'dbus-1',
            'dlog',
            'glib-2.0',
          ]
        },
      }, {
        'sources/': [['exclude', '_tizen\\.cc$|tizen/']],
        'includes/': [['exclude', '_tizen\\.gypi$|tizen/']],
      }],
      ['extension_host_os == "tv"', { 'defines': ['TIZEN_TV'] } ],
      ['extension_host_os == "wearable"', { 'defines': ['TIZEN_WEARABLE'] } ],
      ['extension_host_os == "mobile"', { 'defines': ['TIZEN_MOBILE'] } ],
      ['extension_host_os == "ivi"', { 'defines': ['TIZEN_IVI'] } ],
      ['extension_host_os == "desktop"', { 'defines': ['GENERIC_DESKTOP'] } ],
      ['extension_build_type== "Debug"', {
        'defines': ['_DEBUG', 'TIZEN_DEBUG_ENABLE', ],
        'cflags': [ '-O0', '-g', ],
      }],
      ['extension_build_type == "Release"', {
        'defines': ['NDEBUG', ],
        'cflags': [
          '-O2',
          # Don't emit the GCC version ident directives, they just end up
          # in the .comment section taking up binary size.
          '-fno-ident',
          # Put data and code in their own sections, so that unused symbols
          # can be removed at link time with --gc-sections.
          '-fdata-sections',
          '-ffunction-sections',
        ],
      }],
      [ 'display_type != "wayland"', {
        'sources/': [['exclude', '_wayland\\.cc$|wayland/']],
      }],
      [ 'display_type != "x11"', {
        'sources/': [['exclude', '_x11\\.cc$|x11/']],
      }],
    ],
    'includes': [
      'xwalk_js2c.gypi',
      'pkg-config.gypi',
    ],
    'include_dirs': [
      '../',
      '<(SHARED_INTERMEDIATE_DIR)',
    ],
    'sources': [
      'XW_Extension.cc',
    ],
    'cflags': [
      '-std=c++0x',
      '-fPIC',
      '-fvisibility=hidden',
      '-Wall',
    ],
    'libraries' : [
      '-L .',
      '-Wl,-rpath=/usr/lib/tizen-extensions-crosswalk',
    ],
  },
}
