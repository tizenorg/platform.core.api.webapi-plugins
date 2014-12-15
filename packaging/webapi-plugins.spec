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



# These should be provided by platform
%define tizen_profile_mobile   1
%define tizen_profile_wearable 0
%define tizen_profile_tv       0

####################################################################
#       Mobile Profile :  Kiran(SM-Z130H), Redwood(SM-Z910F)       #
####################################################################
%if 0%{?tizen_profile_mobile}

%define tizen_feature_account_support             0
%define tizen_feature_archive_support             0
%define tizen_feature_badge_support               0
%define tizen_feature_bluetooth_support           0
%define tizen_feature_bluetooth_health_support    0
%define tizen_feature_bookmark_support            0
%define tizen_feature_calendar_support            1
%define tizen_feature_callhistory_support         0
%define tizen_feature_contact_support             1
%define tizen_feature_content_support             0
%define tizen_feature_core_api_support            0
%define tizen_feature_datacontrol_support         0
%define tizen_feature_datasync_support            1
%define tizen_feature_download_support            0
%define tizen_feature_exif_support                0
%define tizen_feature_fm_radio_support            0
%define tizen_feature_gamepad_support             0
%define tizen_feature_ham_support                 0
%define tizen_feature_messaging_email_support     0
%define tizen_feature_messaging_support           0
%define tizen_feature_nbs_support                 0
%define tizen_feature_nfc_emulation_support       0
%define tizen_feature_nfc_support                 0
%define tizen_feature_notification_support        0
%define tizen_feature_power_support               0
%define tizen_feature_push_support                0
%define tizen_feature_sap_support                 0
%define tizen_feature_se_support                  0
%define tizen_feature_sensor_support              0
%define tizen_feature_sound_support               0
%define tizen_feature_system_setting_support      0
%define tizen_feature_telephony_support           0
%define tizen_feature_web_setting_support         0
%define tizen_feature_wi_fi_support               0

%else
####################################################################
#       Wearable Profile :  Gear3(Ponte)                           #
####################################################################
%if 0%{?tizen_profile_wearable}

%define tizen_feature_account_support             0
%define tizen_feature_archive_support             0
%define tizen_feature_badge_support               0
%define tizen_feature_bluetooth_support           0
%define tizen_feature_bluetooth_health_support    0
%define tizen_feature_bookmark_support            0
%define tizen_feature_calendar_support            0
%define tizen_feature_callhistory_support         0
%define tizen_feature_contact_support             0
%define tizen_feature_content_support             0
%define tizen_feature_core_api_support            0
%define tizen_feature_datacontrol_support         0
%define tizen_feature_datasync_support            0
%define tizen_feature_download_support            0
%define tizen_feature_exif_support                0
%define tizen_feature_fm_radio_support            0
%define tizen_feature_gamepad_support             0
%define tizen_feature_ham_support                 0
%define tizen_feature_messaging_email_support     0
%define tizen_feature_messaging_support           0
%define tizen_feature_nbs_support                 0
%define tizen_feature_nfc_emulation_support       0
%define tizen_feature_nfc_support                 0
%define tizen_feature_notification_support        0
%define tizen_feature_power_support               0
%define tizen_feature_push_support                0
%define tizen_feature_sap_support                 0
%define tizen_feature_se_support                  0
%define tizen_feature_sensor_support              0
%define tizen_feature_sound_support               0
%define tizen_feature_system_setting_support      0
%define tizen_feature_telephony_support           0
%define tizen_feature_web_setting_support         0
%define tizen_feature_wi_fi_support               0

%else
####################################################################
#       TV Profile                                                 #
####################################################################
%if 0%{?tizen_profile_tv}

%define tizen_feature_account_support             0
%define tizen_feature_archive_support             0
%define tizen_feature_badge_support               0
%define tizen_feature_bluetooth_support           0
%define tizen_feature_bluetooth_health_support    0
%define tizen_feature_bookmark_support            0
%define tizen_feature_calendar_support            0
%define tizen_feature_callhistory_support         0
%define tizen_feature_contact_support             0
%define tizen_feature_content_support             0
%define tizen_feature_core_api_support            0
%define tizen_feature_datacontrol_support         0
%define tizen_feature_datasync_support            0
%define tizen_feature_download_support            0
%define tizen_feature_exif_support                1
%define tizen_feature_fm_radio_support            0
%define tizen_feature_gamepad_support             0
%define tizen_feature_ham_support                 0
%define tizen_feature_messaging_email_support     0
%define tizen_feature_messaging_support           0
%define tizen_feature_nbs_support                 0
%define tizen_feature_nfc_emulation_support       0
%define tizen_feature_nfc_support                 0
%define tizen_feature_notification_support        0
%define tizen_feature_power_support               0
%define tizen_feature_push_support                0
%define tizen_feature_sap_support                 0
%define tizen_feature_se_support                  0
%define tizen_feature_sensor_support              0
%define tizen_feature_sound_support               0
%define tizen_feature_system_setting_support      0
%define tizen_feature_telephony_support           0
%define tizen_feature_web_setting_support         1
%define tizen_feature_wi_fi_support               0

%endif # tizen_profile_tv
%endif # tizen_profile_wearable
%endif # tizen_profile_mobile


BuildRequires: ninja
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-power)
BuildRequires: pkgconfig(libpcrecpp)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(email-service)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(message-port)
BuildRequires: pkgconfig(minizip)
BuildRequires: pkgconfig(msg-service)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xrandr)
BuildRequires: python


%if 0%{?tizen_feature_badge_support}
BuildRequires:  pkgconfig(badge)
%endif

%if 0%{?tizen_feature_bookmark_support}
BuildRequires:  pkgconfig(capi-web-bookmark)
%endif

%if 0%{?tizen_feature_calendar_support}
BuildRequires:  pkgconfig(calendar-service2)
%endif

%if 0%{?tizen_feature_contact_support}
BuildRequires:  pkgconfig(contacts-service2)
%endif

%if 0%{?tizen_feature_datasync_support}
BuildRequires: pkgconfig(sync-agent)
%endif

%if 0%{?tizen_feature_exif_support}
BuildRequires:  pkgconfig(libexif)
%endif


%description
Tizen Web APIs implemented.

%prep
%setup -q

%build

export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=. -Dtizen=1 -Dextension_build_type=Debug -Dextension_host_os=%{profile}"
GYP_OPTIONS="$GYP_OPTIONS -Ddisplay_type=x11"

./tools/gyp/gyp $GYP_OPTIONS src/tizen-wrt.gyp

ninja -C out/Default %{?_smp_mflags}

%install

# Extensions.
mkdir -p %{buildroot}%{_libdir}/%{crosswalk_extensions}
install -p -m 644 out/Default/libtizen*.so %{buildroot}%{_libdir}/%{crosswalk_extensions}

%files
%{_libdir}/%{crosswalk_extensions}/libtizen*.so

