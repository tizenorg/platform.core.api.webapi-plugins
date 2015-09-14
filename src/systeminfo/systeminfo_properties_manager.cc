/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "systeminfo/systeminfo_properties_manager.h"

#include <memory>

#include <vconf.h>
#include <vconf-internal-keys.h>
#include <system_info.h>
#include <sensor_internal.h>
#include <system_settings.h>
#include <net_connection.h>
#include <sys/statfs.h>

#include "systeminfo/systeminfo_manager.h"
#include "systeminfo/systeminfo_device_capability.h"
#include "common/scope_exit.h"
#include "systeminfo/systeminfo-utils.h"

namespace extension {
namespace systeminfo {

using common::PlatformResult;
using common::ErrorCode;

namespace {
const std::string kMemoryStateNormal = "NORMAL";
const std::string kMemoryStateWarinig = "WARNING";
const double kDisplayInchToMillimeter = 2.54;
//Battery
const double kRemainingBatteryChargeMax = 100.0;
const int kVconfErrorNone = 0;
//Display
const double kDisplayBrightnessDivideValue = 100;
//Device Orientation
const std::string kOrientationPortraitPrimary = "PORTRAIT_PRIMARY";
const std::string kOrientationPortraitSecondary = "PORTRAIT_SECONDARY";
const std::string kOrientationLandscapePrimary = "LANDSCAPE_PRIMARY";
const std::string kOrientationLandscapeSecondary = "LANDSCAPE_SECONDARY";
//Peripheral
const std::string kVideoOutputString = "isVideoOutputOn";
//Storage
const char* kStorageInternalPath = "/opt/usr/media";
const char* kStorageSdcardPath = "/opt/storage/sdcard";
const std::string kPropertyIdStorage = "STORAGE";
const std::string kTypeUnknown = "UNKNOWN";
const std::string kTypeInternal = "INTERNAL";
const std::string kTypeUsbHost = "USB_HOST";
const std::string kTypeMmc = "MMC";
//Network
enum NetworkType {
  kNone,
  kType2G,
  kType2_5G,
  kType3G,
  kType4G,
  kWifi,
  kEthernet,
  kUnknown
};

const char* kNetworkTypeNone = "NONE";
const char* kNetworkType2G = "2G";
const char* kNetworkType2_5G = "2.5G";
const char* kNetworkType3G = "3G";
const char* kNetworkType4G = "4G";
const char* kNetworkTypeWifi = "WIFI";
const char* kNetworkTypeEthernet = "ETHERNET";
const char* kNetworkTypeUnknown = "UNKNOWN";
//Wifi Network
const std::string kWifiStatusOn = "ON";
const std::string kWifiStatusOff = "OFF";
const int kWifiSignalStrengthDivideValue = 100;
//Cellular Network
const unsigned short kMccDivider = 100;
const char* kConnectionOff = "OFF";
const char* kConnectionOn = "ON";
}

SysteminfoPropertiesManager::SysteminfoPropertiesManager(SysteminfoManager& manager)
    : manager_(manager) {
  LoggerD("Entered");
}

SysteminfoPropertiesManager::~SysteminfoPropertiesManager() {
  LoggerD("Entered");
}

PlatformResult SysteminfoPropertiesManager::GetPropertyValue(const std::string& property,
                                                             bool is_array_type,
                                                             picojson::value* res) {
  LoggerD("Entered getPropertyValue");

  if (!is_array_type) {
    picojson::object& res_obj = res->get<picojson::object>();
    return ReportProperty(property, 0, &res_obj);
  } else {
    picojson::object& array_result_obj = res->get<picojson::object>();
    picojson::array& array = array_result_obj.insert(
        std::make_pair("array", picojson::value(picojson::array()))).
            first->second.get<picojson::array>();

    unsigned long property_count = 0;
    PlatformResult ret = manager_.GetPropertyCount(property, &property_count);
    if (ret.IsError()){
      LoggerD("Property is not available");
      return ret;
    }
    LoggerD("property name: %s", property.c_str());
    LoggerD("available property count: %d", property_count);

    for (size_t i = 0; i < property_count; i++) {
      picojson::value result = picojson::value(picojson::object());
      picojson::object& result_obj = result.get<picojson::object>();

      PlatformResult ret = ReportProperty(property, i, &result_obj);
      if (ret.IsError()){
        return ret;
      }
      array.push_back(result);
    }
    if (property_count == 0) {
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Property with given id is not supported");
    }
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportProperty(const std::string& property, int index,
                                               picojson::object* res_obj) {
  LoggerD("Entered");
  if ("BATTERY" == property){
    return ReportBattery(res_obj);
  } else if ("CPU" == property) {
    return ReportCpu(res_obj);
  } else if ("STORAGE" == property) {
    return ReportStorage(res_obj);
  } else if ("DISPLAY" == property) {
    return ReportDisplay(res_obj);
  } else if ("DEVICE_ORIENTATION" == property) {
    return ReportDeviceOrientation(res_obj);
  } else if ("BUILD" == property) {
    return ReportBuild(res_obj);
  } else if ("LOCALE" == property) {
    return ReportLocale(res_obj);
  } else if ("NETWORK" == property) {
    return ReportNetwork(res_obj, index);
  } else if ("WIFI_NETWORK" == property) {
    return ReportWifiNetwork(res_obj);
  } else if ("ETHERNET_NETWORK" == property) {
    return ReportEthernetNetwork(res_obj);
  } else if ("CELLULAR_NETWORK" == property) {
    return ReportCellularNetwork(res_obj, index);
  } else if ("SIM" == property) {
    return ReportSim(res_obj, index);
  } else if ("PERIPHERAL" == property) {
    return ReportPeripheral(res_obj);
  } else if ("MEMORY" == property) {
    return ReportMemory(res_obj);
  } else if ("CAMERA_FLASH" == property) {
    return ReportCameraFlash(res_obj, index);
  }
  LoggerD("Property with given id is not supported");
  return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Property with given id is not supported");
}

/// BATTERY
PlatformResult SysteminfoPropertiesManager::ReportBattery(picojson::object* out) {
  LoggerD("Entered");
  int value = 0;
  int ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CAPACITY, &value);
  if (kVconfErrorNone != ret) {
    std::string log_msg = "Platform error while getting battery detail: ";
    LoggerE("%s%d", log_msg.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, (log_msg + std::to_string(ret)));
  }

  out->insert(std::make_pair("level", picojson::value(static_cast<double>(value)/kRemainingBatteryChargeMax)));
  value = 0;
  ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &value);
  if (kVconfErrorNone != ret) {
    std::string log_msg =  "Platform error while getting battery charging: ";
    LoggerE("%s%d",log_msg.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, (log_msg + std::to_string(ret)));
  }
  out->insert(std::make_pair("isCharging", picojson::value(0 != value)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// CPU
PlatformResult SysteminfoPropertiesManager::ReportCpu(picojson::object* out) {
  LoggerD("Entered");
  static CpuInfo cpu_info;
  FILE *fp = nullptr;
  fp = fopen("/proc/stat", "r");
  if (nullptr == fp) {
    std::string error_msg("Can not open /proc/stat for reading");
    LoggerE( "%s", error_msg.c_str() );
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }

  long long usr = 0;
  long long system = 0;
  long long nice = 0;
  long long idle = 0;
  double load = 0;

  int read_ret = fscanf( fp, "%*s %lld %lld %lld %lld", &usr, &system, &nice, &idle);
  fclose(fp);

  if (4 == read_ret) {
    long long total = usr + nice + system + idle - cpu_info.usr - cpu_info.nice -
        cpu_info.system - cpu_info.idle;
    long long diff_idle = idle - cpu_info.idle;
    if (( total > 0LL ) && ( diff_idle > 0LL )) {
      load = static_cast< double >( diff_idle ) * 100LL / total;
      cpu_info.usr = usr;
      cpu_info.system = system;
      cpu_info.nice = nice;
      cpu_info.idle = idle;
      cpu_info.load = load;
    } else {
      LoggerW("Cannot calculate cpu load, previous value returned");
      load = cpu_info.load;
    }
  } else {
    std::string error_msg( "Could not read /proc/stat" );
    LoggerE( "%s", error_msg.c_str() );
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }

  manager_.SetCpuInfoLoad(cpu_info.load);

  load = 100 - load;
  LoggerD("Cpu load : %f", load );
  out->insert(std::make_pair("load", picojson::value(load / 100.0)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// DISPLAY
PlatformResult SysteminfoPropertiesManager::ReportDisplay(picojson::object* out) {
  LoggerD("Entered");
  int screenWidth = 0;
  int screenHeight = 0;
  int dotsPerInchWidth = 0;
  int dotsPerInchHeight = 0;
  double physicalWidth = 0;
  double physicalHeight = 0;
  double scaledBrightness;

  // FETCH RESOLUTION
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_platform_int(
      "tizen.org/feature/screen.width", &screenWidth)) {
    LoggerE("Cannot get value of screen width");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get value of screen width");
  }
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_platform_int(
      "tizen.org/feature/screen.height", &screenHeight)) {
    LoggerE("Cannot get value of screen height");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get value of screen height");
  }

  //FETCH DOTS PER INCH
  int dots_per_inch=0;
  if (SYSTEM_INFO_ERROR_NONE == system_info_get_platform_int(
      "tizen.org/feature/screen.dpi", &dots_per_inch)) {
    dotsPerInchWidth = dots_per_inch;
    dotsPerInchHeight = dots_per_inch;
  } else {
    LoggerE("Cannot get 'tizen.org/feature/screen.dpi' value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Cannot get 'tizen.org/feature/screen.dpi' value");
  }

  //FETCH PHYSICAL WIDTH
  if (dotsPerInchWidth != 0 && screenWidth != 0) {
    physicalWidth = (screenWidth / dotsPerInchWidth) * kDisplayInchToMillimeter;
  } else {
    std::string log_msg = "Failed to get physical screen width value";
    LoggerE("%s, screenWidth : %d, dotsPerInchWidth: %d", log_msg.c_str(),
         screenWidth, dotsPerInchWidth);
  }

  //FETCH PHYSICAL HEIGHT
  if (dotsPerInchHeight != 0 && screenHeight != 0) {
    physicalHeight = (screenHeight / dotsPerInchHeight) * kDisplayInchToMillimeter;
  } else {
    std::string log_msg = "Failed to get physical screen height value";
    LoggerE("%s, screenHeight : %d, dotsPerInchHeight: %d", log_msg.c_str(),
         screenHeight, dotsPerInchHeight);
  }

  //FETCH BRIGHTNESS
  int brightness;
  if (kVconfErrorNone == vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness)) {
    scaledBrightness = static_cast<double>(brightness)/kDisplayBrightnessDivideValue;
  } else {
    LoggerE("Cannot get brightness value of display");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get brightness value of display");
  }

  out->insert(std::make_pair("resolutionWidth", picojson::value(std::to_string(screenWidth))));
  out->insert(std::make_pair("resolutionHeight", picojson::value(std::to_string(screenHeight))));
  out->insert(std::make_pair("dotsPerInchWidth", picojson::value(std::to_string(dotsPerInchWidth))));
  out->insert(std::make_pair("dotsPerInchHeight", picojson::value(std::to_string(dotsPerInchHeight))));
  out->insert(std::make_pair("physicalWidth", picojson::value(std::to_string(physicalWidth))));
  out->insert(std::make_pair("physicalHeight", picojson::value(std::to_string(physicalHeight))));
  out->insert(std::make_pair("brightness", picojson::value(scaledBrightness)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// DEVICE_ORIENTATION
PlatformResult SysteminfoPropertiesManager::FetchIsAutoRotation(bool* result) {
  LoggerD("Entered");
  int is_auto_rotation = 0;

  if ( 0 == vconf_get_bool(
      VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, &is_auto_rotation)) {
    if (is_auto_rotation) {
      *result = true;
    } else {
      *result = false;
    }
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  else {
    LoggerE("VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
  }
}

PlatformResult SysteminfoPropertiesManager::FetchStatus(std::string* result) {
  LoggerD("Entered");
  int rotation = 0;
  std::string status = kOrientationPortraitPrimary;

  sensor_data_t data;
  bool ret = sensord_get_data(manager_.GetSensorHandle(),
                             AUTO_ROTATION_BASE_DATA_SET, &data);
  if (ret) {
    LoggerD("size of the data value array:%d", data.value_count);
    if (data.value_count > 0 ) {
      rotation = data.values[0];
      LoggerD("rotation is: %d", rotation);
    } else {
      LoggerE("Failed to get data : the size of array is 0. Default rotation would be returned.");
    }
  } else {
    LoggerE("Failed to get data(sensord_get_data). Default rotation would be returned.");
  }


  switch (rotation) {
    case AUTO_ROTATION_DEGREE_UNKNOWN:
    case AUTO_ROTATION_DEGREE_0:
      LoggerD("AUTO_ROTATION_DEGREE_0");
      status = kOrientationPortraitPrimary;
      break;
    case AUTO_ROTATION_DEGREE_90:
      LoggerD("AUTO_ROTATION_DEGREE_90");
      status = kOrientationLandscapePrimary;
      break;
    case AUTO_ROTATION_DEGREE_180:
      LoggerD("AUTO_ROTATION_DEGREE_180");
      status = kOrientationPortraitSecondary;
      break;
    case AUTO_ROTATION_DEGREE_270:
      LoggerD("AUTO_ROTATION_DEGREE_270");
      status = kOrientationLandscapeSecondary;
      break;
    default:
      LoggerE("Received unexpected data: %u", rotation);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Received unexpected data");
  }
  *result = status;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportDeviceOrientation(picojson::object* out) {
  LoggerD("Entered");
  bool is_auto_rotation = false;
  std::string status = "";

  PlatformResult ret = FetchIsAutoRotation(&is_auto_rotation);
  if (ret.IsError()) return ret;

  ret = FetchStatus(&status);
  if (ret.IsError()) return ret;

  out->insert(std::make_pair("isAutoRotation", picojson::value(is_auto_rotation)));
  out->insert(std::make_pair("status", picojson::value(status)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// BUILD
PlatformResult SysteminfoPropertiesManager::ReportBuild(picojson::object* out) {
  LoggerD("Entered");
  std::string model = "";
  PlatformResult ret = SystemInfoDeviceCapability::GetValueString(
      "tizen.org/system/model_name", &model);
  if (ret.IsError()) {
    return ret;
  }
  std::string manufacturer = "";
  ret = SystemInfoDeviceCapability::GetValueString(
      "tizen.org/system/manufacturer", &manufacturer);
  if (ret.IsError()) {
    return ret;
  }
  std::string buildVersion = "";
  ret = SystemInfoDeviceCapability::GetValueString(
      "tizen.org/system/build.string", &buildVersion);
  if (ret.IsError()) {
    return ret;
  }

  out->insert(std::make_pair("model", picojson::value(model)));
  out->insert(std::make_pair("manufacturer", picojson::value(manufacturer)));
  out->insert(std::make_pair("buildVersion", picojson::value(buildVersion)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// LOCALE
PlatformResult SysteminfoPropertiesManager::ReportLocale(picojson::object* out) {
  LoggerD("Entered");
  std::string str_language = "";
  PlatformResult ret = SysteminfoUtils::GetRuntimeInfoString(
      SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &str_language);
  if (ret.IsError()) {
    return ret;
  }

  std::string str_country = "";
  ret = SysteminfoUtils::GetRuntimeInfoString(SYSTEM_SETTINGS_KEY_LOCALE_COUNTRY, &str_country);
  if (ret.IsError()) {
    return ret;
  }

  out->insert(std::make_pair("language", picojson::value(str_language)));
  out->insert(std::make_pair("country", picojson::value(str_country)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// NETWORK
static PlatformResult GetNetworkTypeString(NetworkType type, std::string& type_string) {
  LoggerD("Entered");
  switch (type) {
    case kNone:
      type_string = kNetworkTypeNone;
      break;
    case kType2G:
      type_string = kNetworkType2G;
      break;
    case kType2_5G:
      type_string = kNetworkType2_5G;
      break;
    case kType3G:
      type_string = kNetworkType3G;
      break;
    case kType4G:
      type_string = kNetworkType4G;
      break;
    case kWifi:
      type_string = kNetworkTypeWifi;
      break;
    case kEthernet:
      type_string = kNetworkTypeEthernet;
      break;
    case kUnknown:
      type_string = kNetworkTypeUnknown;
      break;
    default:
      LoggerE("Incorrect type: %d", type);
      return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Incorrect type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportNetwork(picojson::object* out, unsigned long count) {
  LoggerD("Entered with index property %d", count);
  connection_h connection_handle = nullptr;
  connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
  int networkType = 0;
  NetworkType type = kNone;

  //connection must be created in every call, in other case error occurs
  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int(*)(connection_h)>
  connection_handle_ptr(connection_handle, &connection_destroy);
  // automatically release the memory

  error = connection_get_type(connection_handle, &connection_type);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot get connection type: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  switch (connection_type) {
    case CONNECTION_TYPE_DISCONNECTED :
      type = kNone;
      break;
    case CONNECTION_TYPE_WIFI :
      type =  kWifi;
      break;
    case CONNECTION_TYPE_CELLULAR :
      if (TAPI_API_SUCCESS == tel_get_property_int(manager_.GetTapiHandles()[count],
                                                   TAPI_PROP_NETWORK_SERVICE_TYPE, &networkType)) {
        if (networkType < TAPI_NETWORK_SERVICE_TYPE_2G) {
          type =  kNone;
        } else if (networkType == TAPI_NETWORK_SERVICE_TYPE_2G) {
          type =  kType2G;
        } else if (networkType == TAPI_NETWORK_SERVICE_TYPE_2_5G
            || networkType == TAPI_NETWORK_SERVICE_TYPE_2_5G_EDGE) {
          type =  kType2_5G;
        } else if (networkType == TAPI_NETWORK_SERVICE_TYPE_3G
            || networkType == TAPI_NETWORK_SERVICE_TYPE_HSDPA) {
          type =  kType3G;
        } else if (networkType == TAPI_NETWORK_SERVICE_TYPE_LTE) {
          type =  kType4G;
        } else {
          type =  kNone;
        }
      }
      break;
    case CONNECTION_TYPE_ETHERNET :
      type =  kEthernet;
      break;
    default:
      LoggerE("Incorrect type: %d", connection_type);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Incorrect type");
  }
  std::string type_str = "";
  PlatformResult ret = GetNetworkTypeString(type, type_str);
  if(ret.IsError()) {
    return ret;
  }
  out->insert(std::make_pair("networkType", picojson::value(type_str)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// WIFI_NETWORK
static PlatformResult GetIpsWifi(wifi_ap_h wifi_ap_handle, std::string* ip_addr_str,
                   std::string* ipv6_addr_str) {
  LoggerD("Entered");
  //getting ipv4 address
  char* ip_addr = nullptr;
  int error = wifi_ap_get_ip_address(wifi_ap_handle,
                                     WIFI_ADDRESS_FAMILY_IPV4,
                                     &ip_addr);
  if (WIFI_ERROR_NONE != error) {
    LoggerE("Failed to get ip address: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get ip address");
  }
  *ip_addr_str = ip_addr;
  free(ip_addr);

  //getting ipv6 address
  ip_addr = nullptr;
  error = wifi_ap_get_ip_address(wifi_ap_handle,
                                 WIFI_ADDRESS_FAMILY_IPV6,
                                 &ip_addr);
  if (WIFI_ERROR_NONE != error) {
    LoggerE("Failed to get ipv6 address: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get ipv6 address");
  }
  *ipv6_addr_str = ip_addr;
  free(ip_addr);
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// CELLULAR_NETWORK and ETHERNET_NETWORK
static PlatformResult GetIpsFromProfile(connection_profile_h profile_handle, std::string* ip_addr_str,
                             std::string* ipv6_addr_str){
  LoggerD("Entered");
  //getting ipv4 address
  char* ip_addr = nullptr;
  int error = connection_profile_get_ip_address(profile_handle,
                                                CONNECTION_ADDRESS_FAMILY_IPV4,
                                                &ip_addr);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to get ip address: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get ip address");
  }
  *ip_addr_str = ip_addr;
  free(ip_addr);
  //getting ipv6 address
  ip_addr = nullptr;
  error = connection_profile_get_ip_address(profile_handle,
                                            CONNECTION_ADDRESS_FAMILY_IPV6,
                                            &ip_addr);
  if (CONNECTION_ERROR_NONE == error) {
    *ipv6_addr_str = ip_addr;
    free(ip_addr);
  } else if (CONNECTION_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED != error) {
    //core api returns error -97 = CONNECTION_ERROR_ADDRESS_FAMILY_NOT_SUPPORTED
    //it will be supported in the future. For now let's ignore this error
    LoggerE("Failed to get ipv6 address: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get ipv6 address");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportWifiNetwork(picojson::object* out) {
  LoggerD("Entered");

  bool result_status = false;
  std::string result_ssid;
  std::string result_ip_address;
  std::string result_ipv6_address;
  std::string result_mac_address;
  double result_signal_strength = 0;

  // wifi_initialize() must be called in each thread
  int error = wifi_initialize();
  if (WIFI_ERROR_NONE != error) {
    std::string log_msg = "Initialize failed: " + std::string(get_error_message(error));
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  } else {
    LoggerD("WIFI initialization succeed");
  }
  SCOPE_EXIT {
    wifi_deinitialize();
  };

  // check if wifi activated
  bool activated = false;
  error = wifi_is_activated(&activated);
  if (WIFI_ERROR_NONE != error) {
    std::string log_msg = "Checking if wifi is activated failed: " +
        std::string(get_error_message(error));
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  } else {
    LoggerD("WIFI activated check succeed");
  }

  wifi_ap_h wifi_ap_handle = nullptr;
  if (activated) {
    LoggerD("Wifi is activated");
    error = wifi_get_connected_ap(&wifi_ap_handle);
    if (WIFI_ERROR_NONE != error) {
      LoggerD("Error while wifi_get_connnected_ap: %s", get_error_message(error));
      // in case of no connection, ignore error and leave status as false
      if (WIFI_ERROR_NO_CONNECTION != error) {
        std::string log_msg = "Cannot get connected access point handle: " +
            std::string(get_error_message(error));
        LoggerE("%s", log_msg.c_str());
        return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
      }
    } else {
      //if getting connected AP succeed, set status on true
      result_status = true;
    }
  }

  if (result_status) {
    std::unique_ptr<std::remove_pointer<wifi_ap_h>::type, int(*)(wifi_ap_h)>
    wifi_ap_handle_ptr(wifi_ap_handle, &wifi_ap_destroy);
    // automatically release the memory

    //gathering mac address
    char* mac = nullptr;
    error = wifi_get_mac_address(&mac);
    if (WIFI_ERROR_NONE == error && nullptr != mac) {
      SLoggerD("MAC address fetched: %s", mac);
      result_mac_address = mac;
      free(mac);
    } else {
      std::string log_msg = "Failed to get mac address: " + std::string(get_error_message(error));
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }

    //refreshing access point information
    error = wifi_ap_refresh(wifi_ap_handle);
    if (WIFI_ERROR_NONE != error) {
      std::string log_msg = "Failed to refresh access point information: " +
          std::string(get_error_message(error));
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }

    //gathering ssid
    char* essid = nullptr;
    error = wifi_ap_get_essid(wifi_ap_handle, &essid);
    if (WIFI_ERROR_NONE == error) {
      result_ssid = essid;
      free(essid);
    } else {
      std::string log_msg = "Failed to get network ssid: " + std::string(get_error_message(error));
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }

    //gathering ips
    PlatformResult ret = GetIpsWifi(wifi_ap_handle, &result_ip_address, &result_ipv6_address);
    if (ret.IsError()) {
      return ret;
    }

    //gathering strength
    wifi_rssi_level_e rssi_level = manager_.GetWifiLevel();
    // this mean that level was not initialized or wifi not connected
    if (WIFI_RSSI_LEVEL_0 == rssi_level) {
      // so try to gather rssi level with dedicated function
      int rssi = 0;
      error = wifi_ap_get_rssi(wifi_ap_handle, &rssi);
      if (WIFI_ERROR_NONE == error) {
        result_signal_strength = ((double) abs(rssi))/kWifiSignalStrengthDivideValue;
      } else {
        std::string log_msg = "Failed to get signal strength: " +
            std::string(get_error_message(error));
        LoggerE("%s", log_msg.c_str());
        return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
      }
    } else {
      result_signal_strength = ((double) rssi_level)/WIFI_RSSI_LEVEL_4;
    }
  }
  //building result object
  out->insert(std::make_pair("status", picojson::value(result_status ? kWifiStatusOn : kWifiStatusOff)));
  out->insert(std::make_pair("ssid", picojson::value(result_ssid)));
  out->insert(std::make_pair("ipAddress", picojson::value(result_ip_address)));
  out->insert(std::make_pair("ipv6Address", picojson::value(result_ipv6_address)));
  out->insert(std::make_pair("macAddress", picojson::value(result_mac_address)));
  out->insert(std::make_pair("signalStrength", picojson::value(std::to_string(result_signal_strength))));

  return PlatformResult(ErrorCode::NO_ERROR);
}

/// ETHERNET_NETWORK
PlatformResult SysteminfoPropertiesManager::ReportEthernetNetwork(picojson::object* out) {
  LoggerD("Entered");

  std::string result_cable;
  std::string result_status;
  std::string result_ip_address;
  std::string result_ipv6_address;
  std::string result_mac_address;

  connection_h connection_handle = nullptr;
  connection_ethernet_state_e connection_state = CONNECTION_ETHERNET_STATE_DEACTIVATED;
  connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
  connection_profile_h profile_handle = nullptr;

  // connection must be created in every call, in other case error occurs
  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int (*)(connection_h)> connection_handle_ptr(
      connection_handle, &connection_destroy);  // automatically release the memory

  error = connection_get_ethernet_state(connection_handle, &connection_state);
  if (CONNECTION_ERROR_NONE != error) {
    if (CONNECTION_ERROR_NOT_SUPPORTED == error) {
      std::string log_msg = "Cannot get ethernet connection state: Not supported";
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
    }
    std::string log_msg = "Cannot get ethernet connection state: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  switch (connection_state) {
    case CONNECTION_ETHERNET_STATE_DEACTIVATED:
      result_status = "DEACTIVATED";
      break;

    case CONNECTION_ETHERNET_STATE_DISCONNECTED:
      result_status = "DISCONNECTED";
      break;

    case CONNECTION_ETHERNET_STATE_CONNECTED:
      result_status = "CONNECTED";
      break;

    default:
      result_status = "UNKNOWN";
      break;
  }

  connection_ethernet_cable_state_e cable_state = CONNECTION_ETHERNET_CABLE_DETACHED;
  error = connection_get_ethernet_cable_state(connection_handle, &cable_state);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot get ethernet cable state: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  switch (cable_state) {
    case CONNECTION_ETHERNET_CABLE_DETACHED:
      result_cable = "DETACHED";
      break;

    case CONNECTION_ETHERNET_CABLE_ATTACHED:
      result_cable = "ATTACHED";
      break;

    default:
      result_cable = "UNKNOWN";
      break;
  }

  char* mac = nullptr;
  error = connection_get_mac_address(connection_handle, CONNECTION_TYPE_ETHERNET, &mac);
  if (CONNECTION_ERROR_NONE == error && nullptr != mac) {
    SLoggerD("MAC address fetched: %s", mac);
    result_mac_address = mac;
    free(mac);
  } else {
    std::string log_msg = "Failed to get mac address: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  error = connection_get_type(connection_handle, &connection_type);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot get connection type: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  if (CONNECTION_TYPE_ETHERNET == connection_type) {
    //gathering profile
    error = connection_get_current_profile(connection_handle, &profile_handle);
    if (CONNECTION_ERROR_NONE != error) {
      std::string log_msg = "Cannot get connection profile: " + std::to_string(error);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
    std::unique_ptr<std::remove_pointer<connection_profile_h>::type,
        int (*)(connection_profile_h)> profile_handle_ptr(
        profile_handle, &connection_profile_destroy); // automatically release the memory

    //gathering ips
    PlatformResult ret = GetIpsFromProfile(profile_handle, &result_ip_address,
                                           &result_ipv6_address);
    if (ret.IsError()) {
      return ret;
    }
  } else {
    LoggerD("Connection type = %d. ETHERNET is disabled", connection_type);
  }

  out->insert(std::make_pair("cable", picojson::value(result_cable)));
  out->insert(std::make_pair("status", picojson::value(result_status)));
  out->insert(std::make_pair("ipAddress", picojson::value(result_ip_address)));
  out->insert(std::make_pair("ipv6Address", picojson::value(result_ipv6_address)));
  out->insert(std::make_pair("macAddress", picojson::value(result_mac_address)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult FetchBasicSimProperties(TapiHandle *tapi_handle,
    unsigned short *result_mcc,
    unsigned short *result_mnc,
    unsigned short *result_cell_id,
    unsigned short *result_lac,
    bool *result_is_roaming,
    bool *result_is_flight_mode) {
  LoggerD("Entered");
  int result_value = 0;
  int tapi_res = TAPI_API_SUCCESS;
  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_PLMN, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get mcc value, error: " + std::to_string(tapi_res);
    LoggerE("%s", error_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_mcc = static_cast<unsigned short>(result_value) / kMccDivider;
  *result_mnc = static_cast<unsigned short>(result_value) % kMccDivider;

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_CELLID, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get cell_id value, error: " + std::to_string(tapi_res);
    LoggerE("%s", error_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_cell_id = static_cast<unsigned short>(result_value);

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_LAC, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get lac value, error: " + std::to_string(tapi_res);
    LoggerE("%s", error_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_lac = static_cast<unsigned short>(result_value);

  tapi_res = tel_get_property_int(tapi_handle, TAPI_PROP_NETWORK_ROAMING_STATUS, &result_value);
  if (TAPI_API_SUCCESS != tapi_res) {
    std::string error_msg = "Cannot get is_roaming value, error: " + std::to_string(tapi_res);
    LoggerE("%s", error_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }
  *result_is_roaming = (0 != result_value) ? true : false;

  if (0 != vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &result_value)) {
    LoggerE("Cannot get is_flight_mode value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get is_flight_mode value");
  }
  *result_is_flight_mode = (0 != result_value) ? true : false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult FetchConnection(TapiHandle *tapi_handle, std::string* result_status,
                            std::string* result_apn, std::string* result_ip_address,
                            std::string* result_ipv6_address, std::string* result_imei) {
  LoggerD("Entered");
  connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
  connection_profile_h profile_handle = nullptr;
  connection_h connection_handle = nullptr;

  //connection must be created in every call, in other case error occurs
  int error = connection_create(&connection_handle);
  if (CONNECTION_ERROR_NONE != error) {
    std::string log_msg = "Cannot create connection: " + std::to_string(error);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  std::unique_ptr<std::remove_pointer<connection_h>::type, int(*)(connection_h)>
  connection_handle_ptr(connection_handle, &connection_destroy);
  // automatically release the memory

  error = connection_get_type(connection_handle, &connection_type);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to get connection type: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get connection type");
  }

  char* apn = nullptr;
  if (CONNECTION_TYPE_CELLULAR == connection_type) {
    *result_status = kConnectionOn;

    error = connection_get_current_profile(connection_handle,
                                           &profile_handle);
    std::unique_ptr
    <std::remove_pointer<connection_profile_h>::type, int(*)(connection_profile_h)>
    profile_handle_ptr(profile_handle, &connection_profile_destroy);
    // automatically release the memory
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to get profile: %d", error);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get profile");
    }

    error = connection_profile_get_cellular_apn(profile_handle, &apn);
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to get apn name: %d", error);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get apn name");
    }
    *result_apn = apn;
    free(apn);

    PlatformResult ret = GetIpsFromProfile(profile_handle, result_ip_address,
                                           result_ipv6_address);
    if (ret.IsError()) {
      return ret;
    }
  } else {
    *result_status = kConnectionOff;

    //According to previous implementation in case of error
    //don't throw exception here
    error = connection_get_default_cellular_service_profile(
        connection_handle,
        CONNECTION_CELLULAR_SERVICE_TYPE_INTERNET,
        &profile_handle);
    std::unique_ptr
    <std::remove_pointer<connection_profile_h>::type, int(*)(connection_profile_h)>
    profile_handle_ptr(profile_handle, &connection_profile_destroy);
    // automatically release the memory
    if (CONNECTION_ERROR_NONE == error) {
      error = connection_profile_get_cellular_apn(profile_handle, &apn);
      if (CONNECTION_ERROR_NONE == error) {
        *result_apn = apn;
        free(apn);
      } else {
        LoggerE("Failed to get default apn name: %d. Failing silently",
             error);
      }
    } else {
      LoggerE("Failed to get default profile: %d. Failing silently",
           error);
    }
  }

  char* imei = nullptr;
  imei = tel_get_misc_me_imei_sync(tapi_handle);
  if (nullptr != imei) {
    *result_imei = imei;
    free(imei);
  } else {
    LoggerE("Failed to get imei, nullptr pointer. Setting empty value.");
    *result_imei = "";
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportCellularNetwork(picojson::object* out,
                                                                  unsigned long count) {
  LoggerD("Entered with index property %d", count);
  PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
  if (ret.IsError()) {
    return ret;
  }
  std::string result_status;
  std::string result_apn;
  std::string result_ip_address;
  std::string result_ipv6_address;
  unsigned short result_mcc;
  unsigned short result_mnc;
  unsigned short result_cell_id;
  unsigned short result_lac;
  bool result_is_roaming;
  bool result_is_flight_mode;
  std::string result_imei;

  //gathering vconf-based values
  ret = FetchBasicSimProperties(manager_.GetTapiHandles()[count], &result_mcc,
                           &result_mnc, &result_cell_id, &result_lac,
                           &result_is_roaming, &result_is_flight_mode);
  if (ret.IsError()) {
    return ret;
  }
  //gathering connection informations
  ret = FetchConnection(manager_.GetTapiHandles()[count],
                  &result_status, &result_apn, &result_ip_address, &result_ipv6_address,
                  &result_imei);
  if (ret.IsError()) {
    return ret;
  }

  out->insert(std::make_pair("status", picojson::value(result_status)));
  out->insert(std::make_pair("apn", picojson::value(result_apn)));
  out->insert(std::make_pair("ipAddress", picojson::value(result_ip_address)));
  out->insert(std::make_pair("ipv6Address", picojson::value(result_ipv6_address)));
  out->insert(std::make_pair("mcc", picojson::value(std::to_string(result_mcc))));
  out->insert(std::make_pair("mnc", picojson::value(std::to_string(result_mnc))));
  out->insert(std::make_pair("cellId", picojson::value(std::to_string(result_cell_id))));
  out->insert(std::make_pair("lac", picojson::value(std::to_string(result_lac))));
  out->insert(std::make_pair("isRoaming", picojson::value(result_is_roaming)));
  out->insert(std::make_pair("isFligthMode", picojson::value(result_is_flight_mode)));
  out->insert(std::make_pair("imei", picojson::value(result_imei)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// SIM
PlatformResult SysteminfoPropertiesManager::ReportSim(picojson::object* out, unsigned long count) {
  LoggerD("Entered");
  PlatformResult ret = SysteminfoUtils::CheckTelephonySupport();
  if (ret.IsError()) {
    return ret;
  }
  return sim_manager_.GatherSimInformation(
      manager_.GetTapiHandles()[count], out);
}

/// PERIPHERAL
PlatformResult SysteminfoPropertiesManager::ReportPeripheral(picojson::object* out) {
  LoggerD("Entered");
/*  int wireless_display_status = 0;
  PlatformResult ret = GetVconfInt(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &wireless_display_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_MIRACAST_WFD_SOURCE_ON == wireless_display_status) {
      out.insert(std::make_pair(kVideoOutputString, picojson::value(true)));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }*/
  int hdmi_status = 0;
  PlatformResult ret = SysteminfoUtils::GetVconfInt(VCONFKEY_SYSMAN_HDMI, &hdmi_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_SYSMAN_HDMI_CONNECTED == hdmi_status) {
      out->insert(std::make_pair(kVideoOutputString, picojson::value(true)));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  out->insert(std::make_pair(kVideoOutputString, picojson::value(false)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/// MEMORY
PlatformResult SysteminfoPropertiesManager::ReportMemory(picojson::object* out) {
  LoggerD("Entered");
  std::string state = kMemoryStateNormal;
  int status = 0;
  PlatformResult ret = SysteminfoUtils::GetVconfInt(VCONFKEY_SYSMAN_LOW_MEMORY, &status);
  if (ret.IsSuccess()) {
    switch (status) {
      case VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING:
      case VCONFKEY_SYSMAN_LOW_MEMORY_HARD_WARNING:
        state = kMemoryStateWarinig;
        break;
      case VCONFKEY_SYSMAN_LOW_MEMORY_NORMAL:
      default:
        state = kMemoryStateNormal;
    }
  }

  out->insert(std::make_pair("state", picojson::value(state)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void CreateStorageInfo(const std::string& type, struct statfs& fs, picojson::object* out) {
  LoggerD("Entered");
  out->insert(std::make_pair("type", picojson::value(type)));
  out->insert(std::make_pair("capacity", picojson::value(std::to_string(
      static_cast<unsigned long long>(fs.f_bsize) *
      static_cast<unsigned long long>(fs.f_blocks)))));
  out->insert(std::make_pair("availableCapacity", picojson::value(std::to_string(
      static_cast<unsigned long long>(fs.f_bsize) *
      static_cast<unsigned long long>(fs.f_bavail)))));
  bool isRemovable = (type == kTypeInternal) ? false : true;
  out->insert(std::make_pair("isRemovable", picojson::value(isRemovable)));
}

PlatformResult SysteminfoPropertiesManager::ReportStorage(picojson::object* out) {
  LoggerD("Entered");
  int sdcardState = 0;
  struct statfs fs;

  picojson::value result = picojson::value(picojson::array());

  picojson::array& array = result.get<picojson::array>();
  array.push_back(picojson::value(picojson::object()));
  picojson::object& internal_obj = array.back().get<picojson::object>();

  if (statfs(kStorageInternalPath, &fs) < 0) {
    LoggerE("There are no storage units detected");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "There are no storage units detected");
  }
  CreateStorageInfo(kTypeInternal, fs, &internal_obj);
  manager_.SetAvailableCapacityInternal(fs.f_bavail);

  if (0 == vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcardState)) {
    if (VCONFKEY_SYSMAN_MMC_MOUNTED == sdcardState){
      if (statfs(kStorageSdcardPath, &fs) < 0) {
        LoggerE("MMC mounted, but not accessible");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "MMC mounted, but not accessible");
      }
      array.push_back(picojson::value(picojson::object()));
      picojson::object& external_obj = array.back().get<picojson::object>();
      CreateStorageInfo(kTypeMmc, fs, &external_obj);
      manager_.SetAvailableCapacityMmc(fs.f_bavail);
    }
  }

  out->insert(std::make_pair("storages", picojson::value(result)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoPropertiesManager::ReportCameraFlash(picojson::object* out,
                                                              unsigned long index) {
  LoggerD("Entered");
  if (index < manager_.GetCameraTypesCount()) {
    std::string camera = manager_.GetCameraTypes(index);
    out->insert(std::make_pair("camera", picojson::value(camera)));
  } else {
    return PlatformResult(
        ErrorCode::NOT_SUPPORTED_ERR,
        "Camera is not supported on this device");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}


} // namespace systeminfo
} // namespace webapi
