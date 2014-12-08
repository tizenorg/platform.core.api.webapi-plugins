%define profile mobile

%define _manifestdir %{TZ_SYS_RW_PACKAGES}
%define _desktop_icondir %{TZ_SYS_SHARE}/icons/default/small

%define crosswalk_extensions tizen-extensions-crosswalk

Name:       webapi-plugins
Version:    0.1
Release:    0
License:    BSD-3-Clause and Apache-2.0
Group:      Development/Libraries
Summary:    Tizen Web APIs implemented
Source0:    %{name}-%{version}.tar.gz

BuildRequires: ninja
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-power)
BuildRequires: pkgconfig(libpcrecpp)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(message-port)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xrandr)
BuildRequires: python

%description
Tizen Web APIs implemented.

%prep
%setup -q

%build

export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=. -Dtizen=1 -Dextension_build_type=Debug -Dextension_host_os=%{profile}"
GYP_OPTIONS="$GYP_OPTIONS -Ddisplay_type=x11"

./tools/gyp/gyp $GYP_OPTIONS tizen-wrt.gyp

ninja -C out/Default %{?_smp_mflags}

%install

# Extensions.
mkdir -p %{buildroot}%{_libdir}/%{crosswalk_extensions}
install -p -m 644 out/Default/libtizen*.so %{buildroot}%{_libdir}/%{crosswalk_extensions}

%files
%{_libdir}/%{crosswalk_extensions}/libtizen*.so

