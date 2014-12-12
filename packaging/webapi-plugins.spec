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

####################################################################
#       Mobile Profile :  Kiran(SM-Z130H), Redwood(SM-Z910F)       #
####################################################################
%if "%{?profile}" == "mobile"

%define tizen_feature_account_support             0
%define tizen_feature_archive_support             0
%define tizen_feature_badge_support               0
%define tizen_feature_bluetooth_support           0
%define tizen_feature_bluetooth_health_support    0
%define tizen_feature_bookmark_support            1
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
%define tizen_feature_messaging_email_support     1
%define tizen_feature_messaging_support           1
%define tizen_feature_nbs_support                 0
%define tizen_feature_nfc_emulation_support       0
%define tizen_feature_nfc_support                 1
%define tizen_feature_notification_support        1
%define tizen_feature_power_support               1
%define tizen_feature_push_support                0
%define tizen_feature_sap_support                 0
%define tizen_feature_se_support                  1
%define tizen_feature_sensor_support              0
%define tizen_feature_sound_support               0
%define tizen_feature_system_setting_support      1
%define tizen_feature_telephony_support           0
%define tizen_feature_web_setting_support         0
%define tizen_feature_wi_fi_support               0

%else
####################################################################
#       Wearable Profile :  Gear3(Ponte)                           #
####################################################################
%if "%{?profile}" == "wearable"

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

%endif # tizen_profile_wearable
%endif # tizen_profile_mobile


####################################################################
#       TV Profile                                                 #
####################################################################
%if "%{?profile}" == "tv"

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
%define tizen_feature_tvaudio_support             1
%define tizen_feature_web_setting_support         1
%define tizen_feature_wi_fi_support               0
%define tizen_feature_tv_display_support          1
%define tizen_feature_tvchannel_support           1

%endif # tizen_profile_tv

BuildRequires: ninja
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(libpcrecpp)
BuildRequires: pkgconfig(dbus-1)
BuildRequires: pkgconfig(dbus-glib-1)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(capi-message-port)
BuildRequires: pkgconfig(minizip)
BuildRequires: pkgconfig(zlib)
BuildRequires: pkgconfig(msg-service)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(badge)
BuildRequires: pkgconfig(x11)
BuildRequires: pkgconfig(xrandr)
BuildRequires: pkgconfig(ecore)
BuildRequires: python
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-data-control)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(libpcrecpp)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(wrt-plugins-ipc-message)


%if 0%{?tizen_feature_power_support}
BuildRequires: pkgconfig(capi-system-power)
%endif

%if 0%{?tizen_feature_tv_display_support}
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(vconf)
%endif

%if 0%{?tizen_feature_bookmark_support}
BuildRequires: pkgconfig(bookmark-adaptor)
%endif

%if 0%{?tizen_feature_messaging_support}
BuildRequires:  pkgconfig(email-service)
%endif

%if 0%{?tizen_feature_badge_support}
BuildRequires:  pkgconfig(badge)
%endif

%if 0%{?tizen_feature_bookmark_support}
BuildRequires:  pkgconfig(capi-web-bookmark)
BuildRequires: pkgconfig(bookmark-adaptor)
%endif

%if 0%{?tizen_feature_calendar_support}
BuildRequires:  pkgconfig(calendar-service2)
%endif

%if 0%{?tizen_feature_contact_support}
BuildRequires:  pkgconfig(contacts-service2)
%endif

%if 0%{?tizen_feature_datasync_support}
BuildRequires: pkgconfig(sync-agent)
Requires: sync-agent
%endif

%if 0%{?tizen_feature_tvchannel_support}
BuildRequires: pkgconfig(tvs-api)
%endif

%if 0%{?tizen_feature_exif_support}
BuildRequires:  pkgconfig(libexif)
%endif

%if 0%{?tizen_feature_nfc_support}
BuildRequires:  pkgconfig(capi-network-nfc)
%endif

%if 0%{?tizen_feature_fm_radio_support}
BuildRequires: pkgconfig(capi-media-radio)
%endif

%if 0%{?tizen_feature_tvaudio_support}
BuildRequires:  pkgconfig(libavoc)
BuildRequires:  pkgconfig(capi-media-audio-io)
%endif

%if 0%{?tizen_feature_se_support}
BuildRequires:  pkgconfig(smartcard-service)
BuildRequires:  pkgconfig(smartcard-service-common)
%endif

%if 0%{?tizen_feature_notification_support}
BuildRequires: pkgconfig(notification)
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

%if 0%{?tizen_feature_tvaudio_support}
# tv audio beep files:
%define ringtones_directory /opt/usr/share/settings/Ringtones/
mkdir -p %{buildroot}%{ringtones_directory}
cp res/tvsounds/*.pcm %{buildroot}%{ringtones_directory}
%endif


%files
%{_libdir}/%{crosswalk_extensions}/libtizen*.so
%if 0%{?tizen_feature_tvaudio_support}
# tv audio beep files:
%{ringtones_directory}/*.pcm
%endif
