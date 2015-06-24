## Devel package

After build process webapi-plugins-devel-xxx.rpm should be generated in gbs directory.
Package contains required common headers files, gypi files and webapi-plugins.pc file.

### Package structure
```sh
|-usr
|---include
|-----webapi-plugins
|-------src
|---------common
|-------tools
|---------gyp
|-----------pylib
|-------------gyp
|---------------generator
|---lib
|-----pkgconfig
```

File webapi-plugins.pc source:
```
project_name=webapi-plugins
dirname=tizen-extensions-crosswalk
prefix=/usr
exec_prefix=${prefix}
libdir=${prefix}/lib/${dirname}
includedir=${prefix}/include/${project_name}/src

Name: ${project_name}
Description: ${project_name}
Version:
Requires: dbus-1 dlog glib-2.0
Libs: -L${libdir} -ltizen_common
Cflags: -I${includedir}
```

### Creating custom web device plugins module

To create custom web device plugins module ```webapi-plugins.spec```, ```tizen-wrt.gyp``` and ```src``` files are needed.
Skeleton below shows the required structure of test module.

```sh
├── packaging
│   └── webapi-plugins.spec
└── src
│   ├── test
│   ├── test_api.js
│   ├── test_extension.cc
│   ├── test_extension.h
│   ├── test.gyp
│   ├── test_instance.cc
│   └── test_instance.h
└── tizen-wrt.gyp
```

webapi-plugins.spec source:
```
%define _manifestdir %{TZ_SYS_RW_PACKAGES}
%define _desktop_icondir %{TZ_SYS_SHARE}/icons/default/small

%define crosswalk_extensions tizen-extensions-crosswalk

Name:       webapi-plugins-test
Version:    0.1
Release:    0
License:    Apache-2.0 and BSD-2.0 and MIT
Group:      Development/Libraries
Summary:    Tizen Web APIs implemented
Source0:    %{name}-%{version}.tar.gz

BuildRequires: ninja
BuildRequires: pkgconfig(webapi-plugins)

%description
Tizen Test Web APIs.

%prep
%setup -q

%build

export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=. -Dtizen=1 -Dextension_build_type=Debug -Dextension_host_os=%{tizen_profile_name} -Dprivilege_engine=%{tizen_privilege_engine}"
GYP_OPTIONS="$GYP_OPTIONS -Ddisplay_type=x11"

/usr/include/webapi-plugins/tools/gyp/gyp $GYP_OPTIONS src/tizen-wrt.gyp

ninja -C out/Default %{?_smp_mflags}

%install
mkdir -p %{buildroot}%{_libdir}/%{crosswalk_extensions}
install -p -m 644 out/Default/libtizen*.so %{buildroot}%{_libdir}/%{crosswalk_extensions}

%files
%{_libdir}/%{crosswalk_extensions}/libtizen*.so
```

tizen-wrt.gyp source:
```
{
  'includes':[
    '/usr/include/webapi-plugins/src/common/common.gypi',
  ],

  'targets': [
    {
      'target_name': 'extensions',
      'type': 'none',
      'dependencies': [
        'test/test.gyp:*',
      ],
      'conditions': [],
    },
  ],
}
```

test.gyp source:
```
{
  'includes':[
    '/usr/include/webapi-plugins/src/common/common.gypi',
  ],
  'targets': [
    {
      'target_name': 'tizen_test',
      'type': 'loadable_module',
      'sources': [
        'test_api.js',
        'test_extension.cc',
        'test_extension.h',
        'test_instance.cc',
        'test_instance.h',
      ],
      'include_dirs': [
        '../',
        '<(SHARED_INTERMEDIATE_DIR)',
      ],
      'variables': {
        'packages': [
          'webapi-plugins',
        ],
      },
    },
  ],
}
```

[webapi-plugins-devel-test.zip](../src/assets/webapi-plugins-devel-test.zip) contains test module which depends on webapi-plugins devel package.
Custom web device plugins module test is placed in ```src/``` directory and contains all required files.
Please see [Plugin structure](#plugin-structure) chapter for more details.

To install custom web device plugins module ```webapi-plugins-xxx.rpm``` and ```webapi-plugins-devel-xxx.rpm``` must be installed first.
After build and installation webapi-plugins-devel-test ```tizen.test``` namespace should be available.

```javascript
var test = tizen.test.ping();
console.log(test); // Hello!
```
