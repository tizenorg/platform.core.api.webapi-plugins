{
    'target_defaults': {
        'variables': {'packages': ['dlog']},
        'includes': [
            '../common/pkg-config.gypi'
        ]
    },
    'targets' : [
        {
            'target_name': 'desc_gentool',
            'cflags': [
                '-std=c++0x',
                '-Wall'
            ],
            'link_settings': {'libraries': [ '-ldl'], },
            'include_dirs': [
                '../'
            ],
            'type': 'executable',
            'sources': [
                'desc_gentool.cc'
            ]
        }
    ]
}
