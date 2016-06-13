%bcond_with wayland

%{!?profile:%define profile tv}

%define _manifestdir %{TZ_SYS_RW_PACKAGES}
%define _desktop_icondir %{TZ_SYS_SHARE}/icons/default/small

%define crosswalk_extensions tizen-extensions-crosswalk

%define crosswalk_extensions_path %{_libdir}/%{crosswalk_extensions}

Name:       webapi-plugins
Version:    0.94
Release:    0
License:    Apache-2.0 and BSD-2.0 and MIT
Group:      Development/Libraries
Summary:    Tizen Web APIs implemented
Source0:    %{name}-%{version}.tar.gz

%ifarch %{arm} aarch64
# ARM
%define tizen_is_emulator           0
%else
# I586
%define tizen_is_emulator           1
%endif

####################################################################
#       Mobile Profile :  TM1, Redwood(SM-Z910F), KIRAN(Z130H)          #
####################################################################
%if "%{?profile}" == "mobile"

%define tizen_privilege_engine                        CYNARA

%define tizen_feature_account_support                 1
%define tizen_feature_alarm_support                   1
%define tizen_feature_app_control_settings_support    1
%define tizen_feature_application_support             1
%define tizen_feature_archive_support                 1
%define tizen_feature_badge_support                   1
%if 0%{?tizen_is_emulator}
%define tizen_feature_bluetooth_support               0
%else
%define tizen_feature_bluetooth_support               1
%endif
%define tizen_feature_bookmark_support                1
%define tizen_feature_calendar_support                1
%define tizen_feature_contact_support                 1
%define tizen_feature_content_support                 1
%define tizen_feature_datacontrol_support             1
%define tizen_feature_datasync_support                0
%define tizen_feature_download_support                1
%define tizen_feature_exif_support                    1
%define tizen_feature_feedback_support                1
%define tizen_feature_filesystem_support              1
%define tizen_feature_fm_radio_support                1
%if 0%{?tizen_is_emulator}
%define tizen_feature_ham_support                     1
%else
%define tizen_feature_ham_support                     0
%endif
%define tizen_feature_iotcon_support                  0
%define tizen_feature_location_batch                  0
%define tizen_feature_key_manager_support             1
%define tizen_feature_media_controller_support        1
%if 0%{?tizen_is_emulator}
%define tizen_feature_media_key_support               0
%else
%define tizen_feature_media_key_support               1
%endif
%define tizen_feature_message_port_support            1
%define tizen_feature_messaging_support               1

%if 0%{?tizen_is_emulator}
%define tizen_feature_nfc_emulation_support           0
%define tizen_feature_nfc_support                     1
%else
%define tizen_feature_nfc_emulation_support           0
%define tizen_feature_nfc_support                     0
%endif
%define tizen_feature_notification_support            1
%define tizen_feature_package_support                 1
%define tizen_feature_power_support                   1
%define tizen_feature_preference_support              1
%define tizen_feature_push_support                    1
%if 0%{?tizen_is_emulator}
%define tizen_feature_se_support                      0
%else
%define tizen_feature_se_support                      1
%endif
%define tizen_feature_sensor_support                  1
%define tizen_feature_sound_support                   1
%define tizen_feature_system_info_support             1
%define tizen_feature_system_setting_support          1
%if 0%{?tizen_is_emulator}
%define tizen_feature_telephony_support               0
%else
%define tizen_feature_telephony_support               1
%endif
%define tizen_feature_time_support                    1
%define tizen_feature_web_setting_support             1
%define tizen_feature_widget_service_support          1
%if 0%{?tizen_is_emulator}
%define tizen_feature_wi_fi_support                   0
%else
%define tizen_feature_wi_fi_support                   1
%endif
%define tizen_feature_inputdevice_support             1

%if 0%{?tizen_feature_telephony_support}
%define tizen_feature_callhistory_support             1
%define tizen_feature_nbs_support                     1
%else
%define tizen_feature_callhistory_support             0
%define tizen_feature_nbs_support                     0
%endif

%define tizen_feature_tvinputdevice_support           0

%endif # tizen_profile_mobile

####################################################################
#       Wearable Profile :  B2 / TW1                      #
####################################################################
%if "%{?profile}" == "wearable"

%define tizen_privilege_engine                        CYNARA

# Account API is optional in Tizen Wearable Profile.
%define tizen_feature_account_support                 0

%define tizen_feature_alarm_support                   1
%define tizen_feature_app_control_settings_support    1
%define tizen_feature_application_support             1

# Archive API is optional in Tizen Wearable Profile.
%define tizen_feature_archive_support                 1

# Badge API is mandatory in Tizen Wearable Profile.
%define tizen_feature_badge_support                   1

%if 0%{?tizen_is_emulator}
%define tizen_feature_bluetooth_support               0
%else
%define tizen_feature_bluetooth_support               1
%endif

# Bookmark API is optional in Tizen Wearable Profile.
%define tizen_feature_bookmark_support                0

# Calendar API is mandatory in Tizen Wearable Profile.
%define tizen_feature_calendar_support                0
%define tizen_feature_contact_support                 0
%define tizen_feature_content_support                 1
%define tizen_feature_datacontrol_support             1
%define tizen_feature_datasync_support                0
%define tizen_feature_download_support                1
%define tizen_feature_exif_support                    1
%define tizen_feature_feedback_support                1
%define tizen_feature_filesystem_support              1
%define tizen_feature_fm_radio_support                0
%define tizen_feature_ham_support                     1
%define tizen_feature_iotcon_support                  0
%define tizen_feature_location_batch                  0
%define tizen_feature_media_controller_support        1

# MediayKey API is optional in Tizen Wearable Profile.
# tizen.org/feature/network.bluetooth.audio.media is required for MediayKey API
%if 0%{?tizen_is_emulator}
%define tizen_feature_media_key_support               0
%else
%define tizen_feature_media_key_support               1
%endif
%define tizen_feature_key_manager_support             1
%define tizen_feature_message_port_support            1
%define tizen_feature_messaging_support               0

%if 0%{?tizen_is_emulator}
%define tizen_feature_nfc_emulation_support           0
%define tizen_feature_nfc_support                     0
%else
%define tizen_feature_nfc_emulation_support           0
%define tizen_feature_nfc_support                     1
%endif
%define tizen_feature_notification_support            1
%define tizen_feature_package_support                 1
%define tizen_feature_power_support                   1
%define tizen_feature_preference_support              1
%define tizen_feature_push_support                    1
%if 0%{?tizen_is_emulator}
%define tizen_feature_se_support                      0
%else
%define tizen_feature_se_support                      1
%endif
%define tizen_feature_sensor_support                  1
%define tizen_feature_sound_support                   1
%define tizen_feature_system_info_support             1
%define tizen_feature_system_setting_support          1
%define tizen_feature_telephony_support               0
%define tizen_feature_time_support                    1
%define tizen_feature_web_setting_support             0
%define tizen_feature_widget_service_support          1
%define tizen_feature_wi_fi_support                   1
%define tizen_feature_inputdevice_support             1
%define tizen_feature_tvinputdevice_support           0

#- telephony related APIs
# CallHistory API is optional in Tizen Wearable Profile.
# NetworkBearerSelection API is optional in Tizen Wearable Profile.
%if 0%{?tizen_feature_telephony_support}
%define    tizen_feature_callhistory_support          1
%define    tizen_feature_nbs_support                  1
%else
%define    tizen_feature_callhistory_support          0
%define    tizen_feature_nbs_support                  0
%endif

%endif # tizen_profile_wearable

####################################################################
#       TV Profile                                                 #
####################################################################
%if "%{?profile}" == "tv"

%define tizen_privilege_engine                        CYNARA

%define tizen_feature_account_support                 0
%define tizen_feature_alarm_support                   1
%define tizen_feature_app_control_settings_support    0
%define tizen_feature_application_support             1
%define tizen_feature_archive_support                 1
%define tizen_feature_badge_support                   0
%define tizen_feature_bluetooth_support               0
%define tizen_feature_bookmark_support                0
%define tizen_feature_calendar_support                0
%define tizen_feature_callhistory_support             0
%define tizen_feature_contact_support                 0
%define tizen_feature_content_support                 1
%define tizen_feature_datacontrol_support             1
%define tizen_feature_datasync_support                0
%define tizen_feature_download_support                1
%define tizen_feature_exif_support                    1
%define tizen_feature_feedback_support                0
%define tizen_feature_filesystem_support              1
%define tizen_feature_fm_radio_support                0
%define tizen_feature_ham_support                     0
%define tizen_feature_iotcon_support                  0
%define tizen_feature_key_manager_support             1
%define tizen_feature_media_controller_support        1
%define tizen_feature_media_key_support               1
%define tizen_feature_message_port_support            1
%define tizen_feature_messaging_support               0
%define tizen_feature_nbs_support                     0
%define tizen_feature_nfc_emulation_support           0
%define tizen_feature_nfc_support                     0
%define tizen_feature_notification_support            0
%define tizen_feature_package_support                 1
%define tizen_feature_power_support                   0
%define tizen_feature_preference_support              0
%define tizen_feature_push_support                    0
%define tizen_feature_se_support                      0
%define tizen_feature_sensor_support                  0
%define tizen_feature_sound_support                   0
%define tizen_feature_system_info_support             1
%define tizen_feature_system_setting_support          0
%define tizen_feature_telephony_support               0
%define tizen_feature_time_support                    1
%define tizen_feature_web_setting_support             1
%define tizen_feature_widget_service_support          0
%define tizen_feature_wi_fi_support                   1
%define tizen_feature_inputdevice_support             0
%define tizen_feature_tvinputdevice_support           1

%endif # tizen_profile_tv

BuildRequires: pkgconfig(security-privilege-manager)
BuildRequires: ninja
BuildRequires: pkgconfig(appcore-common)
BuildRequires: pkgconfig(dlog)
BuildRequires: pkgconfig(evas)
BuildRequires: pkgconfig(gio-unix-2.0)
BuildRequires: pkgconfig(gio-2.0)
BuildRequires: pkgconfig(glib-2.0)
BuildRequires: pkgconfig(libudev)
BuildRequires: pkgconfig(minizip)
BuildRequires: pkgconfig(zlib)
BuildRequires: pkgconfig(pkgmgr)
BuildRequires: pkgconfig(pkgmgr-info)
BuildRequires: pkgconfig(pkgmgr-installer)
BuildRequires: pkgconfig(vconf)
BuildRequires: pkgconfig(ecore)
BuildRequires: pkgconfig(icu-i18n)
BuildRequires: pkgconfig(aul)
BuildRequires: pkgconfig(storage)
BuildRequires: python
BuildRequires: pkgconfig(capi-base-common)
BuildRequires: pkgconfig(capi-system-info)
BuildRequires: pkgconfig(capi-system-runtime-info)
BuildRequires: pkgconfig(capi-network-connection)
BuildRequires: pkgconfig(capi-system-device)
BuildRequires: pkgconfig(capi-system-system-settings)
BuildRequires: pkgconfig(capi-network-bluetooth)
BuildRequires: pkgconfig(capi-network-wifi)
BuildRequires: pkgconfig(tapi)
BuildRequires: pkgconfig(libpcrecpp)
BuildRequires: pkgconfig(capi-appfw-application)
BuildRequires: pkgconfig(capi-appfw-app-manager)
BuildRequires: pkgconfig(capi-appfw-package-manager)
BuildRequires: pkgconfig(capi-content-media-content)
BuildRequires: pkgconfig(capi-media-metadata-extractor)
BuildRequires: pkgconfig(libtzplatform-config)

%if %{with wayland}
%define display_type wayland
BuildRequires: pkgconfig(wayland-client)
%else
%define display_type x11
#BuildRequires: pkgconfig(x11)
#BuildRequires: pkgconfig(xrandr)
%endif

%if "%{?tizen_privilege_engine}" == "ACE"
BuildRequires: pkgconfig(security-privilege-checker)
%endif

%if "%{?tizen_privilege_engine}" == "CYNARA"
BuildRequires: pkgconfig(cynara-client)
BuildRequires: pkgconfig(libsmack)
%endif

%if 0%{?tizen_feature_account_support}
BuildRequires: pkgconfig(accounts-svc)
%endif

%if 0%{?tizen_feature_alarm_support}
BuildRequires: pkgconfig(capi-appfw-alarm)
BuildRequires: pkgconfig(alarm-service)
%endif

%if 0%{?tizen_feature_bookmark_support}
BuildRequires: pkgconfig(capi-web-bookmark)
BuildRequires: pkgconfig(bookmark-adaptor)
%endif

%if 0%{?tizen_feature_datacontrol_support}
BuildRequires: pkgconfig(capi-data-control)
%endif

%if 0%{?tizen_feature_download_support}
BuildRequires: pkgconfig(capi-web-url-download)
%endif

%if 0%{?tizen_feature_ham_support}
BuildRequires: pkgconfig(motion)
BuildRequires: pkgconfig(capi-system-sensor)
BuildRequires: pkgconfig(capi-location-manager)
BuildRequires: pkgconfig(sensor)
%endif

%if 0%{?tizen_feature_iotcon_support}
BuildRequires: pkgconfig(iotcon)
%endif

%if 0%{?tizen_feature_power_support}
BuildRequires: pkgconfig(deviced)
%endif

%if 0%{?tizen_feature_power_support}
BuildRequires: pkgconfig(capi-appfw-application)
%endif

%if 0%{?tizen_feature_push_support}
BuildRequires: pkgconfig(push)
%endif

%if 0%{?tizen_feature_key_manager_support}
BuildRequires: pkgconfig(key-manager)
%endif

%if 0%{?tizen_feature_media_controller_support}
BuildRequires: pkgconfig(capi-media-controller)
%endif

%if 0%{?tizen_feature_messaging_support}
BuildRequires:  pkgconfig(ecore-file)
BuildRequires:  pkgconfig(email-service)
BuildRequires:  pkgconfig(msg-service)
BuildRequires:  pkgconfig(db-util)
%endif

%if 0%{?tizen_feature_badge_support}
BuildRequires:  pkgconfig(badge)
%endif

%if 0%{?tizen_feature_calendar_support}
BuildRequires:  pkgconfig(calendar-service2)
%endif

%if 0%{?tizen_feature_contact_support}
BuildRequires:  pkgconfig(contacts-service2)
%endif

%if 0%{?tizen_feature_callhistory_support}
BuildRequires:  pkgconfig(contacts-service2)
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

%if 0%{?tizen_feature_feedback_support}
BuildRequires: pkgconfig(feedback)
%endif

%if 0%{?tizen_feature_se_support}
BuildRequires:  pkgconfig(smartcard-service)
BuildRequires:  pkgconfig(smartcard-service-common)
%endif

%if 0%{?tizen_feature_message_port_support}
BuildRequires: pkgconfig(capi-message-port)
%endif

%if 0%{?tizen_feature_notification_support}
BuildRequires: pkgconfig(notification)
%endif

%if 0%{?tizen_feature_sound_support}
BuildRequires:  pkgconfig(capi-media-sound-manager)
%endif

%if 0%{?tizen_feature_sensor_support}
BuildRequires: pkgconfig(capi-system-sensor)
%endif

%if 0%{?tizen_feature_media_key_support}
BuildRequires:  pkgconfig(capi-system-media-key)
%endif

%if 0%{?tizen_feature_widget_service_support}
BuildRequires: pkgconfig(widget_service)
%endif

%description
Tizen Web APIs implemented.

%package devel
Summary:    webapi-plugins development headers
Group:      Development/Libraries
Requires:   %{name} = %{version}

%description devel
webapi-plugins development headers

%prep
%setup -q

%build

export GYP_GENERATORS='ninja'
GYP_OPTIONS="--depth=. -Dtizen=1 -Dextension_build_type=Debug -Dextension_host_os=%{profile} -Dprivilege_engine=%{tizen_privilege_engine}"
GYP_OPTIONS="$GYP_OPTIONS -Ddisplay_type=%{display_type}"
GYP_OPTIONS="$GYP_OPTIONS -Dcrosswalk_extensions_path=%{crosswalk_extensions_path}"

# feature flags
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_is_emulator=%{?tizen_is_emulator}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_account_support=%{?tizen_feature_account_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_alarm_support=%{?tizen_feature_alarm_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_app_control_settings_support=%{?tizen_feature_app_control_settings_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_application_support=%{?tizen_feature_application_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_archive_support=%{?tizen_feature_archive_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_badge_support=%{?tizen_feature_badge_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_bluetooth_support=%{?tizen_feature_bluetooth_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_bookmark_support=%{?tizen_feature_bookmark_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_calendar_support=%{?tizen_feature_calendar_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_callhistory_support=%{?tizen_feature_callhistory_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_contact_support=%{?tizen_feature_contact_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_content_support=%{?tizen_feature_content_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_datacontrol_support=%{?tizen_feature_datacontrol_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_datasync_support=%{?tizen_feature_datasync_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_download_support=%{?tizen_feature_download_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_exif_support=%{?tizen_feature_exif_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_feedback_support=%{?tizen_feature_feedback_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_filesystem_support=%{?tizen_feature_filesystem_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_fm_radio_support=%{?tizen_feature_fm_radio_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_ham_support=%{?tizen_feature_ham_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_iotcon_support=%{?tizen_feature_iotcon_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_location_batch=%{?tizen_feature_location_batch}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_key_manager_support=%{?tizen_feature_key_manager_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_media_controller_support=%{?tizen_feature_media_controller_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_media_key_support=%{?tizen_feature_media_key_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_message_port_support=%{?tizen_feature_message_port_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_messaging_support=%{?tizen_feature_messaging_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_nbs_support=%{?tizen_feature_nbs_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_nfc_emulation_support=%{?tizen_feature_nfc_emulation_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_nfc_support=%{?tizen_feature_nfc_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_notification_support=%{?tizen_feature_notification_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_package_support=%{?tizen_feature_package_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_power_support=%{?tizen_feature_power_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_preference_support=%{?tizen_feature_preference_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_push_support=%{?tizen_feature_push_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_sap_support=%{?tizen_feature_sap_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_sensor_support=%{?tizen_feature_sensor_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_se_support=%{?tizen_feature_se_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_sound_support=%{?tizen_feature_sound_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_system_info_support=%{?tizen_feature_system_info_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_system_setting_support=%{?tizen_feature_system_setting_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_telephony_support=%{?tizen_feature_telephony_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_time_support=%{?tizen_feature_time_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_inputdevice_support=%{?tizen_feature_inputdevice_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_web_setting_support=%{?tizen_feature_web_setting_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_widget_service_support=%{?tizen_feature_widget_service_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_wi_fi_support=%{?tizen_feature_wi_fi_support}"
GYP_OPTIONS="$GYP_OPTIONS -Dtizen_feature_tvinputdevice_support=%{?tizen_feature_tvinputdevice_support}"

./tools/gyp/gyp $GYP_OPTIONS src/tizen-wrt.gyp

ninja -C out/Default %{?_smp_mflags}

%install
mkdir -p %{buildroot}/usr/share/license
cp LICENSE %{buildroot}/usr/share/license/%{name}
cat LICENSE.BSD-2.0 >> %{buildroot}/usr/share/license/%{name}

# Extensions.
mkdir -p %{buildroot}%{crosswalk_extensions_path}
install -p -m 644 out/Default/libtizen*.so %{buildroot}%{crosswalk_extensions_path}

# devel files
mkdir -p %{buildroot}%{_libdir}/pkgconfig
sed -i s,__LIB_DIR__,%{crosswalk_extensions_path},g packaging/%{name}.pc.in
sed -i s,__INCLUDE_DIR__,%{_includedir}/%{name}/src,g packaging/%{name}.pc.in
cp packaging/%{name}.pc.in %{buildroot}%{_libdir}/pkgconfig/%{name}.pc
mkdir -p %{buildroot}%{_includedir}/%{name}/src/common
install -p -m 644 src/common/*.h %{buildroot}%{_includedir}/%{name}/src/common
install -p -m 644 src/common/*.gypi %{buildroot}%{_includedir}/%{name}/src/common
install -p -m 644 src/common/XW_Extension.cc %{buildroot}%{_includedir}/%{name}/src/common
mkdir -p %{buildroot}%{_includedir}/%{name}/tools
install -p -m 644 tools/generate_api.py %{buildroot}%{_includedir}/%{name}/tools
install -p -m 644 tools/mergejs.py %{buildroot}%{_includedir}/%{name}/tools
install -p -m 644 tools/js_minimize.py %{buildroot}%{_includedir}/%{name}/tools
cp -a tools/gyp %{buildroot}%{_includedir}/%{name}/tools/gyp
cp -a tools/slimit %{buildroot}%{_includedir}/%{name}/tools/slimit
cp -a out/Default/desc_gentool %{buildroot}%{_includedir}/%{name}/tools/desc_gentool

# execute desc_gentool
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:%{buildroot}%{crosswalk_extensions_path} out/Default/desc_gentool \
	%{crosswalk_extensions_path} \
	%{buildroot}%{crosswalk_extensions_path} > plugins.json

# temporary plugins description for lazy loading
install -p -m 644 plugins.json %{buildroot}%{crosswalk_extensions_path}/plugins.json


%files
%{crosswalk_extensions_path}/libtizen*.so
%{crosswalk_extensions_path}/plugins.json
%{_datadir}/license/%{name}
%manifest webapi-plugins.manifest

%files devel
%{_includedir}/*
%{_libdir}/pkgconfig/*
