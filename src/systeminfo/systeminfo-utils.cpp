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

#include "systeminfo-utils.h"
#include "systeminfo/systeminfo_instance.h"

#include <fstream>
#include <sstream>
#include <memory>
#include <mutex>

#include <runtime_info.h>
#include <system_info.h>
#include <sys/statfs.h>

#include <vconf.h>
#include <net_connection.h>
#include <tapi_common.h>
#include <ITapiModem.h>
#include <ITapiSim.h>
#include <device.h>
#include <device/callback.h>
#include <device/device-error.h>
#include <sensor_internal.h>
#include <tzplatform_config.h>

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/dbus_operation.h"

//Hardcoded values for Kiran Device
#define FEATURE_OPTIONAL_TELEPHONY 1
#define FEATURE_OPTIONAL_BT 1
#define ENABLE_KIRAN 1
#define FEATURE_OPTIONAL_WI_FI 1
#define PROFILE_MOBILE 1

#define DUID_KEY_STRING 28
#define DUID_BUFFER_SIZE 100
#define SEED_LENGTH 16
#define CRYPT_KEY_SIZE 8

#define TAPI_HANDLE_MAX 2
#define DEFAULT_PROPERTY_COUNT 1

#ifdef FEATURE_OPTIONAL_WI_FI
#include <wifi.h>
#endif

#ifdef FEATURE_OPTIONAL_TELEPHONY
#include <ITapiModem.h>
#endif

#ifdef FEATURE_OPTIONAL_BT
#include <bluetooth.h>
#endif


namespace extension {
namespace systeminfo {

namespace {
const std::string MEMORY_STATE_NORMAL = "NORMAL";
const std::string MEMORY_STATE_WARNING = "WARNING";
const int MEMORY_TO_BYTE = 1024;
const int BASE_GATHERING_INTERVAL = 100;
const double DISPLAY_INCH_TO_MILLIMETER = 2.54;
}
using namespace common;

//Callback functions declarations
static void OnBatteryChangedCb(keynode_t* node, void* event_ptr);
static gboolean OnCpuChangedCb(gpointer event_ptr);
static gboolean OnStorageChangedCb(gpointer event_ptr);
static void OnMmcChangedCb(keynode_t* node, void* event_ptr);
static void OnDisplayChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceOrientationChangedCb(sensor_t sensor, unsigned int event_type,
                                         sensor_data_t *data, void *user_data);
static void OnLocaleChangedCb(runtime_info_key_e key, void* event_ptr);
static void OnNetworkChangedCb(connection_type_e type, void* event_ptr);
static void OnNetworkValueChangedCb(const char* ipv4_address,
                                    const char* ipv6_address, void* event_ptr);
static void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr);
static void OnPeripheralChangedCb(keynode_t* node, void* event_ptr);
static void OnMemoryChangedCb(keynode_t* node, void* event_ptr);
static void OnBrightnessChangedCb(device_callback_e type, void *value, void *user_data);

static void SimCphsValueCallback(TapiHandle *handle, int result, void *data, void *user_data);
static void SimMsisdnValueCallback(TapiHandle *handle, int result, void *data, void *user_data);
static void SimSpnValueCallback(TapiHandle *handle, int result, void *data, void *user_data);

namespace {
// device profile
const char* kPlatformFull = "mobile-full";
const char* kPlatformMobile = "mobile-web";
const char* kPlatformWearable = "wearable";

const char* kProfileFull = "MOBILE_FULL";
const char* kProfileMobile = "MOBILE_WEB";
const char* kProfileWearable = "WEARABLE";
//opengles
const char* kOpenglesTextureDelimiter = "/";
const char* kOpenglesTextureUtc = "utc";
const char* kOpenglesTexturePtc = "ptc";
const char* kOpenglesTextureEtc = "etc";
const char* kOpenglesTexture3dc = "3dc";
const char* kOpenglesTextureAtc = "atc";
const char* kOpenglesTexturePvrtc = "pvrtc";
//core cpu arch
const char* kPlatformCoreDelimiter = " | ";
const char* kPlatformCoreArmv6 = "armv6";
const char* kPlatformCoreArmv7 = "armv7";
const char* kPlatformCoreX86 = "x86";
//core fpu arch
const char* kPlatformCoreSse2 = "sse2";
const char* kPlatformCoreSse3 = "sse3";
const char* kPlatformCoreSsse3 = "ssse3";
const char* kPlatformCoreVfpv2 = "vfpv2";
const char* kPlatformCoreVfpv3 = "vfpv3";
const std::string kPropertyIdCpu = "CPU";
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
const std::string kStorageInternalPath = tzplatform_getenv(TZ_USER_CONTENT);
const std::string kStorageSdcardPath = std::string(tzplatform_getenv(TZ_SYS_STORAGE)) + "/sdcard";
const std::string kPropertyIdStorage = "STORAGE";
const std::string kTypeUnknown = "UNKNOWN";
const std::string kTypeInternal = "INTERNAL";
const std::string kTypeUsbHost = "USB_HOST";
const std::string kTypeMmc = "MMC";
const double kPropertyWatcherTime = 1;
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
//Sim
const char* kSimStatusAbsent = "ABSENT";
const char* kSimStatusInitializing = "INITIALIZING";
const char* kSimStatusReady = "READY";
const char* kSimStatusPinRequired = "PIN_REQUIRED";
const char* kSimStatusPukRequired = "PUK_REQUIRED";
const char* kSimStatusSimLocked = "SIM_LOCKED";
const char* kSimStatusNetworkLocked = "NETWORK_LOCKED";
const char* kSimStatusUnknown = "UNKNOWN";

///*for getCapability*/
/*API feature*/
/*Network feature*/
const char* kTizenFeatureBluetoothAlwaysOn = "http://tizen.org/capability/network.bluetooth.always_on"; //TODO mobile/wearable: false, tv: true
const char* kTizenFeatureOpenglesTextureFormat = "http://tizen.org/feature/opengles.texture_format";
const char* kTizenFeatureCoreApiVersion = "http://tizen.org/feature/platform.core.api.version";
const char* kTizenFeaturePlatfromCoreCpuArch = "http://tizen.org/feature/platform.core.cpu.arch";
const char* kTizenFeaturePlatfromCoreFpuArch = "http://tizen.org/feature/platform.core.fpu.arch";
/*profile feature*/
const char* kTizenFeatureProfile = "http://tizen.org/feature/profile";
/*Screen feature*/
const char* kTizenFeatureScreen = "http://tizen.org/feature/screen";
/*Sensor feature*/
const char* kTizenFeatureCpuFrequency = "http://tizen.org/feature/platform.core.cpu.frequency";
/*platform*/
const char* kTizenFeaturePlatformNativeApiVersion = "tizen.org/feature/platform.native.api.version";
const char* kTizenFeaturePlatformNativeOspCompatible = "tizen.org/feature/platform.native.osp_compatible";
const char* kTizenFeaturePlatformVersionName = "http://tizen.org/feature/platform.version.name";
}

/////////////////////////// SimDetailsManager ////////////////////////////////

class SimDetailsManager {
 private:
  unsigned short mcc_;
  unsigned short mnc_;
  std::string operator_name_;
  std::string msin_;
  std::string state_;
  std::string msisdn_;
  std::string iccid_;
  std::string spn_;

  picojson::object* sim_result_obj_;
  unsigned short to_process_;
  std::mutex sim_to_process_mutex_;
  std::mutex sim_info_mutex_;
  long sim_count_;

  void ResetSimHolder(picojson::object* out);
  void FetchSimState(TapiHandle *tapi_handle);
  PlatformResult FetchSimSyncProps(TapiHandle *tapi_handle);
  void ReturnSimToJS();

 public:
  SimDetailsManager();

  PlatformResult GatherSimInformation(TapiHandle* handle, picojson::object* out);
  long GetSimCount(TapiHandle **tapi_handle);
  void TryReturn();

  void set_operator_name(const std::string& name)
  {
    std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
    operator_name_ = name;
    --to_process_;
    LoggerD("Operator name: %s", operator_name_.c_str());
  };
  void set_msisdn(const std::string& msisdn)
  {
    std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
    this->msisdn_ = msisdn;
    --to_process_;
    LoggerD("MSISDN number: %s", this->msisdn_.c_str());
  };
  void set_spn(const std::string& spn)
  {
    std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
    this->spn_ = spn;
    --to_process_;
    LoggerD("SPN value: %s", this->spn_.c_str());
  };
};

SimDetailsManager::SimDetailsManager():
            mcc_(0),
            mnc_(0),
            operator_name_(""),
            msin_(""),
            state_(""),
            msisdn_(""),
            iccid_(""),
            spn_(""),
            sim_result_obj_(nullptr),
            to_process_(0),
            sim_count_(0)
{
}

PlatformResult SimDetailsManager::GatherSimInformation(TapiHandle* handle, picojson::object* out)
{
  std::lock_guard<std::mutex> first_lock_sim(sim_info_mutex_);
  ResetSimHolder(out);

  FetchSimState(handle);
  if (kSimStatusReady == state_) {
    PlatformResult ret = FetchSimSyncProps(handle);
    if (ret.IsError()) {
      return ret;
    }
    {
      //All props should be fetched synchronously, but sync function does not work
      std::lock_guard<std::mutex> lock_to_process(sim_to_process_mutex_);
      //would be deleted on } ending bracket
      int result = tel_get_sim_cphs_netname(handle, SimCphsValueCallback, nullptr);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting cphs netname: %d", result);
      }

      result = tel_get_sim_msisdn(handle, SimMsisdnValueCallback, nullptr);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting msisdn: %d", result);
      }

      result = tel_get_sim_spn(handle, SimSpnValueCallback, nullptr);
      if (TAPI_API_SUCCESS == result) {
        ++to_process_;
      } else {
        LoggerE("Failed getting spn: %d", result);
      }
    }
    //prevent returning not filled result
    std::lock_guard<std::mutex> lock_sim(sim_info_mutex_);
    //result will come from callbacks
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  //if sim state is not READY return default values and don't wait for callbacks
  TryReturn();
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SimDetailsManager::FetchSimState(TapiHandle *tapi_handle)
{
  LoggerD("Entered");
  if (nullptr == tapi_handle) {
    LoggerE("Tapi handle is null");
    state_ = kSimStatusUnknown;
  } else {
    int card_changed = 0;
    TelSimCardStatus_t sim_card_state;
    int error = tel_get_sim_init_info(tapi_handle, &sim_card_state, &card_changed);
    if (TAPI_API_SUCCESS == error) {
      switch (sim_card_state) {
        case TAPI_SIM_STATUS_CARD_NOT_PRESENT:
        case TAPI_SIM_STATUS_CARD_REMOVED:
          state_ = kSimStatusAbsent;
          break;
        case TAPI_SIM_STATUS_SIM_INITIALIZING:
          state_ = kSimStatusInitializing;
          break;
        case TAPI_SIM_STATUS_SIM_INIT_COMPLETED:
          state_ = kSimStatusReady;
          break;
        case TAPI_SIM_STATUS_SIM_PIN_REQUIRED:
          state_ = kSimStatusPinRequired;
          break;
        case TAPI_SIM_STATUS_SIM_PUK_REQUIRED:
          state_ = kSimStatusPukRequired;
          break;
        case TAPI_SIM_STATUS_SIM_LOCK_REQUIRED:
        case TAPI_SIM_STATUS_CARD_BLOCKED:
          state_ = kSimStatusSimLocked;
          break;
        case TAPI_SIM_STATUS_SIM_NCK_REQUIRED:
        case TAPI_SIM_STATUS_SIM_NSCK_REQUIRED:
          state_ = kSimStatusNetworkLocked;
          break;
        default:
          state_ = kSimStatusUnknown;
          break;
      }
    }
  }
}

PlatformResult SimDetailsManager::FetchSimSyncProps(TapiHandle *tapi_handle)
{
  LoggerD("Entered");
  TelSimImsiInfo_t imsi;
  int error = tel_get_sim_imsi(tapi_handle, &imsi);
  if (TAPI_API_SUCCESS == error) {
    LoggerD("mcc: %s, mnc: %s, msin: %s", imsi.szMcc, imsi.szMnc, imsi.szMsin);
    mcc_ = std::stoul(imsi.szMcc);
    mnc_ = std::stoul(imsi.szMnc);
    msin_ = imsi.szMsin;
  }
  else {
    LoggerE("Failed to get sim imsi: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get sim imsi");
  }

  //TODO add code for iccid value fetching, when proper API would be ready
  iccid_ = "";
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SimDetailsManager::ResetSimHolder(picojson::object* out){
  sim_result_obj_ = out;
  to_process_ = 0;
  mcc_ = 0;
  mnc_ = 0;
  operator_name_ = "";
  msin_ = "";
  state_ = "";
  msisdn_ = "";
  iccid_ = "";
  spn_ = "";
}

void SimDetailsManager::ReturnSimToJS(){
  LoggerD("Entered");
  if (nullptr != sim_result_obj_) {
    sim_result_obj_->insert(std::make_pair("state", picojson::value(state_)));
    sim_result_obj_->insert(std::make_pair("operatorName", picojson::value(operator_name_)));
    sim_result_obj_->insert(std::make_pair("msisdn", picojson::value(msisdn_)));
    sim_result_obj_->insert(std::make_pair("iccid", picojson::value(iccid_)));
    sim_result_obj_->insert(std::make_pair("mcc", picojson::value(std::to_string(mcc_))));
    sim_result_obj_->insert(std::make_pair("mnc", picojson::value(std::to_string(mnc_))));
    sim_result_obj_->insert(std::make_pair("msin", picojson::value(msin_)));
    sim_result_obj_->insert(std::make_pair("spn", picojson::value(spn_)));
    //everything returned, clear pointer
    sim_result_obj_ = nullptr;
  } else {
    LoggerE("No sim returned JSON object pointer is null");
  }
}

long SimDetailsManager::GetSimCount(TapiHandle **tapi_handle){
  if (0 != sim_count_){
    LoggerD("Sim counted already");
  } else {
    LoggerD("Gathering sim count");
    char **cp_list = tel_get_cp_name_list();
    if (cp_list != NULL) {
      while (cp_list[sim_count_]) {
        tapi_handle[sim_count_] = tel_init(cp_list[sim_count_]);
        if (tapi_handle[sim_count_] == NULL) {
          LoggerE("Failed to connect with tapi, handle is null");
          break;
        }
        sim_count_++;
        LoggerD("%d modem: %s", sim_count_, cp_list[sim_count_]);
      }
    } else {
      LoggerE("Failed to get cp list");
      sim_count_ = TAPI_HANDLE_MAX;
    }
    g_strfreev(cp_list);
  }
  return sim_count_;
}

void SimDetailsManager::TryReturn(){
  if (0 == to_process_){
    LoggerD("Returning property to JS");
    ReturnSimToJS();
    sim_info_mutex_.unlock();
  } else {
    LoggerD("Not ready yet - waiting");
  }
}

/////////////////////////// SystemInfoListeners ////////////////////////////////

class SystemInfoListeners {
 public:
  SystemInfoListeners();
  ~SystemInfoListeners();

  PlatformResult RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterBatteryListener();
  PlatformResult RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                     SysteminfoInstance& instance);
  PlatformResult UnregisterCpuListener();
  PlatformResult RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterStorageListener();
  PlatformResult RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterDisplayListener();
  PlatformResult RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                   SysteminfoInstance& instance);
  PlatformResult UnregisterDeviceOrientationListener();
  PlatformResult RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterLocaleListener();
  PlatformResult RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                         SysteminfoInstance& instance);
  PlatformResult UnregisterNetworkListener();
  PlatformResult RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                             SysteminfoInstance& instance);
  PlatformResult UnregisterWifiNetworkListener();
  PlatformResult RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                 SysteminfoInstance& instance);
  PlatformResult UnregisterCellularNetworkListener();
  PlatformResult RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                            SysteminfoInstance& instance);
  PlatformResult UnregisterPeripheralListener();
  PlatformResult RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterMemoryListener();
  PlatformResult RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                        SysteminfoInstance& instance);
  PlatformResult UnregisterCameraFlashListener();

  void SetCpuInfoLoad(double load);
  void SetAvailableCapacityInternal(unsigned long long capacity);
  void SetAvailableCapacityMmc(unsigned long long capacity);

  void OnBatteryChangedCallback(keynode_t* node, void* event_ptr);
  void OnCpuChangedCallback(void* event_ptr);
  void OnStorageChangedCallback(void* event_ptr);
  void OnMmcChangedCallback(keynode_t* node, void* event_ptr);
  void OnDisplayChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceAutoRotationChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceOrientationChangedCallback(sensor_t sensor, unsigned int event_type,
                                          sensor_data_t *data, void *user_data);
  void OnLocaleChangedCallback(runtime_info_key_e key, void* event_ptr);
  void OnNetworkChangedCallback(connection_type_e type, void* event_ptr);
  void OnNetworkValueCallback(const char* ipv4_address,
                              const char* ipv6_address, void* event_ptr);
  void OnCellularNetworkValueCallback(keynode_t *node, void *event_ptr);
  void OnPeripheralChangedCallback(keynode_t* node, void* event_ptr);
  void OnMemoryChangedCallback(keynode_t* node, void* event_ptr);
  void OnBrightnessChangedCallback(device_callback_e type, void* value, void* user_data);

  TapiHandle* GetTapiHandle();
  TapiHandle** GetTapiHandles();
  PlatformResult GetConnectionHandle(connection_h&);
  int GetSensorHandle();
  PlatformResult ConnectSensor(int* result);
  void DisconnectSensor(int handle_orientation);
 private:
  static PlatformResult RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                              SysteminfoInstance& instance);
  static PlatformResult UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb);
  PlatformResult RegisterIpChangeCallback(SysteminfoInstance& instance);
  PlatformResult UnregisterIpChangeCallback();
  void InitTapiHandles();

  guint m_cpu_event_id;
  guint m_storage_event_id;

  double m_cpu_load;
  double m_last_cpu_load;
  unsigned long long m_available_capacity_internal;
  unsigned long long m_available_capacity_mmc;
  unsigned long long m_last_available_capacity_internal;
  unsigned long long m_last_available_capacity_mmc;

  SysteminfoUtilsCallback m_battery_listener;
  SysteminfoUtilsCallback m_cpu_listener;
  SysteminfoUtilsCallback m_storage_listener;
  SysteminfoUtilsCallback m_display_listener;
  SysteminfoUtilsCallback m_device_orientation_listener;
  SysteminfoUtilsCallback m_locale_listener;
  SysteminfoUtilsCallback m_network_listener;
  SysteminfoUtilsCallback m_wifi_network_listener;
  SysteminfoUtilsCallback m_cellular_network_listener;
  SysteminfoUtilsCallback m_peripheral_listener;
  SysteminfoUtilsCallback m_memory_listener;
  SysteminfoUtilsCallback m_camera_flash_listener;

  TapiHandle *m_tapi_handles[TAPI_HANDLE_MAX+1];
  //for ip change callback
  connection_h m_connection_handle;
  //! Sensor handle for DeviceOrientation purposes
  int m_sensor_handle;
};

SystemInfoListeners::SystemInfoListeners():
            m_cpu_event_id(0),
            m_storage_event_id(0),
            m_cpu_load(0),
            m_last_cpu_load(0),
            m_available_capacity_internal(0),
            m_available_capacity_mmc(0),
            m_last_available_capacity_internal(0),
            m_last_available_capacity_mmc(0),
            m_battery_listener(nullptr),
            m_cpu_listener(nullptr),
            m_storage_listener(nullptr),
            m_display_listener(nullptr),
            m_device_orientation_listener(nullptr),
            m_locale_listener(nullptr),
            m_network_listener(nullptr),
            m_wifi_network_listener(nullptr),
            m_cellular_network_listener(nullptr),
            m_peripheral_listener(nullptr),
            m_memory_listener(nullptr),
            m_camera_flash_listener(nullptr),
            m_tapi_handles{nullptr},
            m_connection_handle(nullptr),
            m_sensor_handle(-1)
{
  LoggerD("Entered");
}

SystemInfoListeners::~SystemInfoListeners(){
  LoggerD("Entered");
  UnregisterBatteryListener();
  UnregisterCpuListener();
  UnregisterStorageListener();
  UnregisterDisplayListener();
  UnregisterDeviceOrientationListener();
  UnregisterLocaleListener();
  UnregisterNetworkListener();
  UnregisterWifiNetworkListener();
  UnregisterCellularNetworkListener();
  UnregisterPeripheralListener();
  UnregisterMemoryListener();

  DisconnectSensor(m_sensor_handle);

  unsigned int i = 0;
  while(m_tapi_handles[i]) {
    tel_deinit(m_tapi_handles[i]);
    i++;
  }
  if (nullptr != m_connection_handle) {
    connection_destroy(m_connection_handle);
  }
}

#define CHECK_LISTENER_ERROR(method) \
  ret = method; \
  if (ret.IsError()) { \
    return ret; \
  }

int SystemInfoListeners::GetSensorHandle() {
  if (m_sensor_handle < 0) {
    LoggerD("Connecting to sensor");
    ConnectSensor(&m_sensor_handle);
  } else {
    LoggerD("Sensor already connected");
  }
  return m_sensor_handle;
}

PlatformResult SystemInfoListeners::ConnectSensor(int* result) {
  LoggerD("Entered");
  sensor_t sensor = sensord_get_sensor(AUTO_ROTATION_SENSOR);
  int handle_orientation = sensord_connect(sensor);
  if (handle_orientation < 0) {
    std::string log_msg = "Failed to connect auto rotation sensor";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  bool ret = sensord_start(handle_orientation, 0);
  if(!ret) {
    sensord_disconnect(handle_orientation);
    std::string log_msg = "Failed to start auto rotation sensor";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  LoggerD("Sensor starts successfully = %d", handle_orientation);
  *result = handle_orientation;
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SystemInfoListeners::DisconnectSensor(int handle_orientation)
{
  if (handle_orientation >= 0) {
    LoggerD("Entered");
    bool state = sensord_stop(handle_orientation);
    LoggerD("sensord_stop() returned state = %d", state);
    state = sensord_disconnect(handle_orientation);
    LoggerD("sensord_disconnect() returned state %d", state);
  } else {
    LoggerD("sensor already disconnected - no action needed");
  }
}

PlatformResult SystemInfoListeners::RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  LoggerD("Entered");
  if (nullptr == m_battery_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb, instance))
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb, instance))
    LoggerD("Added callback for BATTERY");
    m_battery_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterBatteryListener()
{
  if (nullptr != m_battery_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb))
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb))
    LoggerD("Removed callback for BATTERY");
    m_battery_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  if (nullptr == m_cpu_listener) {
    m_cpu_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnCpuChangedCb, static_cast<void*>(&instance));
    LoggerD("Added callback for CPU");
    m_cpu_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCpuListener()
{
  if (nullptr != m_cpu_listener) {
    g_source_remove(m_cpu_event_id);
    m_cpu_event_id = 0;
    LoggerD("Removed callback for CPU");
    m_cpu_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_storage_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb, instance))

    m_storage_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnStorageChangedCb, static_cast<void*>(&instance));
    LoggerD("Added callback for STORAGE");
    m_storage_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterStorageListener()
{
  if (nullptr != m_storage_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb))

    g_source_remove(m_storage_event_id);
    m_storage_event_id = 0;
    LoggerD("Removed callback for STORAGE");
    m_storage_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_display_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb, instance))
    LoggerD("Added callback for DISPLAY");
    m_display_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterDisplayListener()
{
  if (nullptr != m_display_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb))
    LoggerD("Removed callback for DISPLAY");
    m_display_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                                      SysteminfoInstance& instance)
{
  if (nullptr == m_device_orientation_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        RegisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb, instance))

    bool sensor_ret = sensord_register_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE,
                                     BASE_GATHERING_INTERVAL, 0,
                                     OnDeviceOrientationChangedCb, static_cast<void*>(&instance));
    if (!sensor_ret) {
      LoggerE("Failed to register orientation change event listener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to register orientation change event listener");
    }

    LoggerD("Added callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterDeviceOrientationListener()
{
  if (nullptr != m_device_orientation_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(
        UnregisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb))
    bool sensor_ret = sensord_unregister_event(GetSensorHandle(), AUTO_ROTATION_EVENT_CHANGE_STATE);
    if (!sensor_ret) {
      LoggerE("Failed to unregister orientation change event listener");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister"
          " orientation change event listener");
    }

    LoggerD("Removed callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  if (nullptr == m_locale_listener) {
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_set_changed_cb(RUNTIME_INFO_KEY_REGION,
                                    OnLocaleChangedCb, static_cast<void*>(&instance)) ) {
      LoggerE("Country change callback registration failed");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Country change callback registration failed");
    }
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_set_changed_cb(RUNTIME_INFO_KEY_LANGUAGE,
                                    OnLocaleChangedCb, static_cast<void*>(&instance)) ) {
      LoggerE("Language change callback registration failed");
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Language change callback registration failed");
    }
    LoggerD("Added callback for LOCALE");
    m_locale_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterLocaleListener()
{
  if (nullptr != m_locale_listener) {
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_LANGUAGE) ) {
      LoggerE("Unregistration of language change callback failed");
    }
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_REGION) ) {
      LoggerE("Unregistration of country change callback failed");
    }
    LoggerD("Removed callback for LOCALE");
    m_locale_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  if (nullptr == m_network_listener) {
    connection_h handle;
    PlatformResult ret(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
    if (CONNECTION_ERROR_NONE !=
        connection_set_type_changed_cb(handle, OnNetworkChangedCb, static_cast<void*>(&instance))) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Registration of listener failed");
    }
    LoggerD("Added callback for NETWORK");
    m_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterNetworkListener()
{
  if (nullptr != m_network_listener) {
    connection_h handle;
    PlatformResult ret(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
    if (CONNECTION_ERROR_NONE != connection_unset_type_changed_cb(handle)) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unregistration of listener failed");
    }
    LoggerD("Removed callback for NETWORK");
    m_network_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance)
{
  if (nullptr == m_wifi_network_listener) {
    if (nullptr == m_cellular_network_listener){
      //register if there is no callback both for wifi and cellular
      PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
      CHECK_LISTENER_ERROR(RegisterIpChangeCallback(instance))
    } else {
      LoggerD("No need to register ip listener on platform, already registered");
    }
    LoggerD("Added callback for WIFI_NETWORK");
    m_wifi_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterWifiNetworkListener()
{
  //unregister if is wifi callback, but no cellular callback
  if (nullptr != m_wifi_network_listener && nullptr == m_cellular_network_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterIpChangeCallback())
    LoggerD("Removed callback for WIFI_NETWORK");
  } else {
    LoggerD("Removed callback for WIFI_NETWORK, but cellular listener still works");
  }
  m_wifi_network_listener = nullptr;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                    SysteminfoInstance& instance)
{
  if (nullptr == m_cellular_network_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    if (nullptr == m_wifi_network_listener){
      //register if there is no callback both for wifi and cellular
      CHECK_LISTENER_ERROR(RegisterIpChangeCallback(instance))
    } else {
      LoggerD("No need to register ip listener on platform, already registered");
    }
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                          OnCellularNetworkValueChangedCb, instance))
    CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                          OnCellularNetworkValueChangedCb, instance))
    LoggerD("Added callback for CELLULAR_NETWORK");
    m_cellular_network_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCellularNetworkListener()
{
  if (nullptr != m_cellular_network_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                            OnCellularNetworkValueChangedCb))
    CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                            OnCellularNetworkValueChangedCb))
    if (nullptr == m_wifi_network_listener) {
      //register if there is no callback both for wifi and cellular
      CHECK_LISTENER_ERROR(UnregisterIpChangeCallback())
      LoggerD("Removed callback for CELLULAR_NETWORK");
    } else {
      LoggerD("Removed callback for CELLULAR_NETWORK, but cellular listener still works");
    }
  }
  m_cellular_network_listener = nullptr;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                                               SysteminfoInstance& instance)
{
  if (nullptr == m_peripheral_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                                 OnPeripheralChangedCb, instance))
    }
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                                 OnPeripheralChangedCb, instance))
    }
    // TODO(r.galka) temporarily removed - not supported by platform
    //if (-1 != vconf_get_int(VCONFKEY_POPSYNC_ACTIVATED_KEY, &value)) {
    //  CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_POPSYNC_ACTIVATED_KEY,
    //                                             OnPeripheralChangedCb, instance))
    //}
    LoggerD("Added callback for PERIPHERAL");
    m_peripheral_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterPeripheralListener()
{
  if (nullptr != m_peripheral_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS,
                                                   OnPeripheralChangedCb))
    }
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_HDMI, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_SYSMAN_HDMI,
                                                   OnPeripheralChangedCb))
    }
    // TODO(r.galka) temporarily removed - not supported by platform
    //if (-1 != vconf_get_int(VCONFKEY_POPSYNC_ACTIVATED_KEY, &value)) {
    //  CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_POPSYNC_ACTIVATED_KEY,
    //                                               OnPeripheralChangedCb))
    //}
    LoggerD("Removed callback for PERIPHERAL");
    m_peripheral_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  if (nullptr == m_memory_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
      CHECK_LISTENER_ERROR(RegisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb, instance))
    }
    LoggerD("Added callback for MEMORY");
    m_memory_listener = callback;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterMemoryListener()
{
  if (nullptr != m_memory_listener) {
    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    int value = 0;
    if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
      CHECK_LISTENER_ERROR(UnregisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb))
    }
    LoggerD("Removed callback for MEMORY");
    m_memory_listener = nullptr;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  // TODO(r.galka) temporarily removed - not supported by platform
  //if (nullptr == m_camera_flash_listener) {
  //  if (DEVICE_ERROR_NONE != device_add_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
  //                            OnBrightnessChangedCb, static_cast<void*>(&instance))) {
  //      return PlatformResult(ErrorCode::UNKNOWN_ERR);
  //    }
  //    m_camera_flash_listener = callback;
  //}
    return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterCameraFlashListener()
{
  // TODO(r.galka) temporarily removed - not supported by platform
  //if (nullptr != m_camera_flash_listener) {
  //  PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
  //  int value = 0;
  //  if (DEVICE_ERROR_NONE != device_remove_callback(DEVICE_CALLBACK_FLASH_BRIGHTNESS,
  //                                               OnBrightnessChangedCb)) {
  //    return PlatformResult(ErrorCode::UNKNOWN_ERR);
  //  }
  //  LoggerD("Removed callback for camera_flash");
  //  m_camera_flash_listener = nullptr;
  //}
  return PlatformResult(ErrorCode::NO_ERROR);
}


void SystemInfoListeners::SetCpuInfoLoad(double load)
{
  m_cpu_load = load;
}

void SystemInfoListeners::SetAvailableCapacityInternal(unsigned long long capacity)
{
  m_available_capacity_internal = capacity;
}

void SystemInfoListeners::SetAvailableCapacityMmc(unsigned long long capacity)
{
  m_available_capacity_mmc = capacity;
}

void SystemInfoListeners::OnBatteryChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_battery_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_battery_listener(*instance);
  }
}

void SystemInfoListeners::OnCpuChangedCallback(void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu, false, result);
  if (ret.IsSuccess()) {
    if (m_cpu_load == m_last_cpu_load) {
      return;
    }
    if (nullptr != m_cpu_listener) {
      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
      m_last_cpu_load = m_cpu_load;
      m_cpu_listener(*instance);
    }
  }
}

void SystemInfoListeners::OnStorageChangedCallback(void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
  if (ret.IsSuccess()) {
    if (m_available_capacity_internal == m_last_available_capacity_internal) {
      return;
    }

    if (nullptr != m_storage_listener) {
      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
      m_last_available_capacity_internal = m_available_capacity_internal;
      m_storage_listener(*instance);
    }
  }
}

void SystemInfoListeners::OnMmcChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
  if (ret.IsSuccess()) {
    if (m_available_capacity_mmc == m_last_available_capacity_mmc) {
      return;
    }
    if (nullptr != m_storage_listener) {
      SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
      m_last_available_capacity_mmc = m_available_capacity_mmc;
      m_storage_listener(*instance);
    }
  }
}


void SystemInfoListeners::OnDisplayChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_display_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_display_listener(*instance);
  }
}

void SystemInfoListeners::OnDeviceAutoRotationChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_device_orientation_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_device_orientation_listener(*instance);
  }
}

void SystemInfoListeners::OnDeviceOrientationChangedCallback(sensor_t sensor, unsigned int event_type,
                                                             sensor_data_t *data, void *user_data)
{
  if (nullptr != m_device_orientation_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(user_data);
    m_device_orientation_listener(*instance);
  }
}

void SystemInfoListeners::OnLocaleChangedCallback(runtime_info_key_e /*key*/, void* event_ptr)
{
  if (nullptr != m_locale_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_locale_listener(*instance);
  }
}

void SystemInfoListeners::OnNetworkChangedCallback(connection_type_e /*type*/, void* event_ptr)
{
  if (nullptr != m_network_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_network_listener(*instance);
  }
}

void SystemInfoListeners::OnNetworkValueCallback(const char* /*ipv4_address*/,
                                                 const char* /*ipv6_address*/, void* event_ptr)
{
  SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
  if (nullptr != m_wifi_network_listener) {
    m_wifi_network_listener(*instance);
  }
  if (nullptr != m_cellular_network_listener) {
    m_cellular_network_listener(*instance);
  }
}

void SystemInfoListeners::OnCellularNetworkValueCallback(keynode_t */*node*/, void *event_ptr)
{
  if (nullptr != m_cellular_network_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_cellular_network_listener(*instance);
  }
}

void SystemInfoListeners::OnPeripheralChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_peripheral_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_peripheral_listener(*instance);
  }
}

void SystemInfoListeners::OnMemoryChangedCallback(keynode_t* /*node*/, void* event_ptr)
{
  if (nullptr != m_memory_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(event_ptr);
    m_memory_listener(*instance);
  }
}

void SystemInfoListeners::OnBrightnessChangedCallback(device_callback_e type, void* value, void* user_data)
{
  if (nullptr != m_camera_flash_listener) {
    SysteminfoInstance* instance = static_cast<SysteminfoInstance*>(user_data);
    m_camera_flash_listener(*instance);
  }
}

void SystemInfoListeners::InitTapiHandles()
{
  LoggerD("Entered");
  int sim_count = 0;
  if (nullptr == m_tapi_handles){
    char **cp_list = tel_get_cp_name_list();
    *m_tapi_handles = nullptr;
    if (nullptr != cp_list) {
      while (cp_list[sim_count]) {
        m_tapi_handles[sim_count] = tel_init(cp_list[sim_count]);
        if (nullptr == m_tapi_handles[sim_count]) {
          LoggerE("Failed to connect with tapi, handle is null");
          break;
        }
        sim_count++;
        LoggerD("%d modem: %s", sim_count, cp_list[sim_count]);
      }
    } else {
      LoggerE("Failed to get cp list");
      sim_count = TAPI_HANDLE_MAX;
    }
    g_strfreev(cp_list);
  }
}

TapiHandle* SystemInfoListeners::GetTapiHandle()
{

  LoggerD("Entered");
  InitTapiHandles();
  return m_tapi_handles[0];
}

TapiHandle** SystemInfoListeners::GetTapiHandles()
{
  InitTapiHandles();
  return m_tapi_handles;
}

PlatformResult SystemInfoListeners::GetConnectionHandle(connection_h& handle)
{
  if (nullptr == m_connection_handle) {
    int error = connection_create(&m_connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to create connection: %d", error);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot create connection");
    }
  }
  handle = m_connection_handle;
  return PlatformResult(ErrorCode::NO_ERROR);
}

//////////////// Private ////////////////////

PlatformResult SystemInfoListeners::RegisterVconfCallback(const char *in_key, vconf_callback_fn cb,
                                                          SysteminfoInstance& instance)
{
  if (0 != vconf_notify_key_changed(in_key, cb, static_cast<void*>(&instance))) {
    LoggerE("Failed to register vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to register vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
  if (0 != vconf_ignore_key_changed(in_key, cb)) {
    LoggerE("Failed to unregister vconf callback: %s", in_key);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to unregister vconf callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::RegisterIpChangeCallback(SysteminfoInstance& instance)
{
  LoggerD("Registering connection callback");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  int error = connection_set_ip_address_changed_cb(handle,
                                                   OnNetworkValueChangedCb,
                                                   static_cast<void*>(&instance));
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to register ip change callback: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot register ip change callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoListeners::UnregisterIpChangeCallback()
{
  LoggerD("Unregistering connection callback");
  connection_h handle;
  PlatformResult ret(ErrorCode::NO_ERROR);
  CHECK_LISTENER_ERROR(GetConnectionHandle(handle))
  int error = connection_unset_ip_address_changed_cb(handle);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to unregister ip change callback: %d", error);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot unregister ip change callback");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}


/////////////////////////// system_info_listeners object ////////////////////////

static SystemInfoListeners system_info_listeners;

//global sim manager - needed for async gathering informations
static SimDetailsManager sim_mgr;

/////////////////// Callbacks ///////////////////////////////////////////////////

void OnBatteryChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnBatteryChangedCallback(node, event_ptr);
}

gboolean OnCpuChangedCb(gpointer event_ptr)
{
  LoggerD("");
  system_info_listeners.OnCpuChangedCallback(event_ptr);
  return G_SOURCE_CONTINUE;
}
gboolean OnStorageChangedCb(gpointer event_ptr)
{
  LoggerD("");
  system_info_listeners.OnStorageChangedCallback(event_ptr);
  return G_SOURCE_CONTINUE;
}
void OnMmcChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnMmcChangedCallback(node, event_ptr);
}

void OnDisplayChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnDisplayChangedCallback(node, event_ptr);
}

void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnDeviceAutoRotationChangedCallback(node, event_ptr);
}

void OnDeviceOrientationChangedCb(sensor_t sensor, unsigned int event_type,
                                  sensor_data_t *data, void *user_data)
{
  LoggerD("");
  system_info_listeners.OnDeviceOrientationChangedCallback(sensor, event_type, data, user_data);
}

void OnLocaleChangedCb(runtime_info_key_e key, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnLocaleChangedCallback(key, event_ptr);
}

void OnNetworkChangedCb(connection_type_e type, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnNetworkChangedCallback(type, event_ptr);
}

void OnNetworkValueChangedCb(const char* ipv4_address,
                             const char* ipv6_address, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnNetworkValueCallback(ipv4_address, ipv6_address, event_ptr);
}

void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr)
{
  LoggerD("");
  system_info_listeners.OnCellularNetworkValueCallback(node, event_ptr);
}

void OnPeripheralChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnPeripheralChangedCallback(node, event_ptr);
}

void OnMemoryChangedCb(keynode_t* node, void* event_ptr)
{
  LoggerD("");
  system_info_listeners.OnMemoryChangedCallback(node, event_ptr);
}

void OnBrightnessChangedCb(device_callback_e type, void* value, void* user_data)
{
  LoggerD("");
  // TODO(r.galka) temporarily removed - not supported by platform
  //if (type == DEVICE_CALLBACK_FLASH_BRIGHTNESS) {
  //  system_info_listeners.OnBrightnessChangedCallback(type, value, user_data);
  //}
}

/////////////////////////// SysteminfoUtils ////////////////////////////////

PlatformResult SystemInfoDeviceCapability::GetValueBool(const char *key, bool* value) {
  bool platform_result = false;
  int ret = system_info_get_platform_bool(key, &platform_result);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    std::string log_msg = "Platform error while getting bool value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *value = platform_result;
  LoggerD("value[%s]: %s", key, value ? "true" : "false");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueInt(const char *key, int* value) {
  int platform_result = 0;
  int ret = system_info_get_platform_int(key, &platform_result);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    std::string log_msg = "Platform error while getting int value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  *value = platform_result;
  LoggerD("value[%s]: %d", key, value);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueString(const char *key, std::string* str_value) {
  char* value = nullptr;

  int ret = system_info_get_platform_string(key, &value);
  if (SYSTEM_INFO_ERROR_NONE == ret) {
    if (value != nullptr) {
      *str_value = value;
      free(value);
      value = nullptr;
    }
  } else {
    std::string log_msg = "Platform error while getting string value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  LoggerD("value[%s]: %s", key, str_value->c_str());
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult GetRuntimeInfoString(runtime_info_key_e key, std::string& platform_string) {
  char* platform_c_string;
  int err = runtime_info_get_value_string(key, &platform_c_string);
  if (RUNTIME_INFO_ERROR_NONE == err) {
    if (nullptr != platform_c_string) {
      platform_string = platform_c_string;
      free(platform_c_string);
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  const char* error_msg = "Error when retrieving runtime information: " + err;
  LoggerE("%s", error_msg);
  return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
}

PlatformResult GetVconfInt(const char *key, int &value) {
  if (0 == vconf_get_int(key, &value)) {
    return PlatformResult(ErrorCode::NO_ERROR);
  } else {
    const std::string error_msg = "Could not get " + std::string(key);
    LoggerD("%s",error_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, error_msg);
  }

  LoggerD("value[%s]: %d", key, value);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetTotalMemory(long long& result)
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_total(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetAvailableMemory(long long& result)
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_available(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  result = static_cast<long long>(value*MEMORY_TO_BYTE);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetCount(const std::string& property, unsigned long& count)
{
  LoggerD("Enter");

  if ("BATTERY" == property || "CPU" == property || "STORAGE" == property ||
      "DISPLAY" == property || "DEVICE_ORIENTATION" == property ||
      "BUILD" == property || "LOCALE" == property || "NETWORK" == property ||
      "WIFI_NETWORK" == property || "CELLULAR_NETWORK" == property ||
      "PERIPHERAL" == property || "MEMORY" == property) {
    count = DEFAULT_PROPERTY_COUNT;
  } else if ("SIM" == property) {
    count = sim_mgr.GetSimCount(system_info_listeners.GetTapiHandles());
  } else if ("CAMERA_FLASH" == property) {
    const int numberOfCameraFlashProperties = 3;
    count = numberOfCameraFlashProperties;
  } else {
    LoggerD("Property with given id is not supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Property with given id is not supported");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportProperty(const std::string& property, int index,
                                               picojson::object& res_obj) {
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
    return ReportNetwork(res_obj);
  } else if ("WIFI_NETWORK" == property) {
    return ReportWifiNetwork(res_obj);
  } else if ("CELLULAR_NETWORK" == property) {
    return ReportCellularNetwork(res_obj);
  } else if ("SIM" == property) {
    return ReportSim(res_obj, index);
  } else if ("PERIPHERAL" == property) {
    return ReportPeripheral(res_obj);
  } else if ("MEMORY" == property) {
    return ReportMemory(res_obj);
  } else if ("CAMERA_FLASH" == property) {
    return ReportCameraFlash(res_obj);
  } else {
    LoggerD("Property with given id is not supported");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Property with given id is not supported");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::GetPropertyValue(const std::string& property, bool is_array_type,
                                                  picojson::value& res)
{
  LoggerD("Entered getPropertyValue");

  if (!is_array_type) {
    picojson::object& res_obj = res.get<picojson::object>();
    return ReportProperty(property, 0, res_obj);
  } else {
    picojson::object& array_result_obj = res.get<picojson::object>();
    picojson::array& array = array_result_obj.insert(
        std::make_pair("array", picojson::value(picojson::array()))).
            first->second.get<picojson::array>();

    unsigned long property_count = 0;
    PlatformResult ret = SysteminfoUtils::GetCount(property, property_count);
    if (ret.IsError()){
      return ret;
    }

    for (int i = 0; i < property_count; i++) {
      picojson::value result = picojson::value(picojson::object());
      picojson::object& result_obj = result.get<picojson::object>();

      ret = ReportProperty(property, i, result_obj);
      if (ret.IsError()){
        return ret;
      }
      array.push_back(result);
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportBattery(picojson::object& out) {
  int value = 0;
  int ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CAPACITY, &value);
  if (kVconfErrorNone != ret) {
    std::string log_msg = "Platform error while getting battery detail: ";
    LoggerE("%s%d", log_msg.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, (log_msg + std::to_string(ret)));
  }

  out.insert(std::make_pair("level", picojson::value(static_cast<double>(value)/kRemainingBatteryChargeMax)));
  value = 0;
  ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &value);
  if (kVconfErrorNone != ret) {
    std::string log_msg =  "Platform error while getting battery charging: ";
    LoggerE("%s%d",log_msg.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, (log_msg + std::to_string(ret)));
  }
  out.insert(std::make_pair("isCharging", picojson::value(0 != value)));
  return PlatformResult(ErrorCode::NO_ERROR);
}
//TODO maybe make two functions later onGSourceFunc
PlatformResult SysteminfoUtils::ReportCpu(picojson::object& out) {
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

  system_info_listeners.SetCpuInfoLoad(cpu_info.load);

  load = 100 - load;
  LoggerD("Cpu load : %f", load );
  out.insert(std::make_pair("load", picojson::value(load / 100.0)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportDisplay(picojson::object& out) {
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
    physicalWidth = (screenWidth / dotsPerInchWidth) * DISPLAY_INCH_TO_MILLIMETER;
  } else {
    std::string log_msg = "Failed to get physical screen width value";
    LoggerE("%s, screenWidth : %d, dotsPerInchWidth: %d", log_msg.c_str(),
         screenWidth, dotsPerInchWidth);
  }

  //FETCH PHYSICAL HEIGHT
  if (dotsPerInchHeight != 0 && screenHeight != 0) {
    physicalHeight = (screenHeight / dotsPerInchHeight) * DISPLAY_INCH_TO_MILLIMETER;
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

  out.insert(std::make_pair("resolutionWidth", picojson::value(std::to_string(screenWidth))));
  out.insert(std::make_pair("resolutionHeight", picojson::value(std::to_string(screenHeight))));
  out.insert(std::make_pair("dotsPerInchWidth", picojson::value(std::to_string(dotsPerInchWidth))));
  out.insert(std::make_pair("dotsPerInchHeight", picojson::value(std::to_string(dotsPerInchHeight))));
  out.insert(std::make_pair("physicalWidth", picojson::value(std::to_string(physicalWidth))));
  out.insert(std::make_pair("physicalHeight", picojson::value(std::to_string(physicalHeight))));
  out.insert(std::make_pair("brightness", picojson::value(scaledBrightness)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult FetchIsAutoRotation(bool* result)
{
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

static PlatformResult FetchStatus(std::string* result)
{
  LoggerD("Entered");
  int rotation = 0;
  std::string status = kOrientationPortraitPrimary;

  sensor_data_t data;
  bool ret = sensord_get_data(system_info_listeners.GetSensorHandle(),
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

PlatformResult SysteminfoUtils::ReportDeviceOrientation(picojson::object& out) {
  bool is_auto_rotation = false;
  std::string status = "";

  PlatformResult ret = FetchIsAutoRotation(&is_auto_rotation);
  if (ret.IsError()) return ret;

  ret = FetchStatus(&status);
  if (ret.IsError()) return ret;

  out.insert(std::make_pair("isAutoRotation", picojson::value(is_auto_rotation)));
  out.insert(std::make_pair("status", picojson::value(status)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportBuild(picojson::object& out) {
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

  out.insert(std::make_pair("model", picojson::value(model)));
  out.insert(std::make_pair("manufacturer", picojson::value(manufacturer)));
  out.insert(std::make_pair("buildVersion", picojson::value(buildVersion)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportLocale(picojson::object& out) {
  std::string str_language = "";
  PlatformResult ret = GetRuntimeInfoString(RUNTIME_INFO_KEY_LANGUAGE, str_language);
  if (ret.IsError()) {
    return ret;
  }

  std::string str_country = "";
  ret = GetRuntimeInfoString(RUNTIME_INFO_KEY_REGION, str_country);
  if (ret.IsError()) {
    return ret;
  }

  out.insert(std::make_pair("language", picojson::value(str_language)));
  out.insert(std::make_pair("country", picojson::value(str_country)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult GetNetworkTypeString(NetworkType type, std::string& type_string)
{
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

PlatformResult SysteminfoUtils::ReportNetwork(picojson::object& out) {
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
      if (vconf_get_int(VCONFKEY_TELEPHONY_SVCTYPE, &networkType) == 0) {
        if (networkType < VCONFKEY_TELEPHONY_SVCTYPE_2G) {
          type =  kNone;
        } else if (networkType == VCONFKEY_TELEPHONY_SVCTYPE_2G) {
          type =  kType2G;
        } else if (networkType == VCONFKEY_TELEPHONY_SVCTYPE_2G
            || networkType == VCONFKEY_TELEPHONY_SVCTYPE_2_5G_EDGE) {
          type =  kType2_5G;
        } else if (networkType == VCONFKEY_TELEPHONY_SVCTYPE_3G
            || networkType == VCONFKEY_TELEPHONY_SVCTYPE_HSDPA) {
          type =  kType3G;
        } else if (networkType == VCONFKEY_TELEPHONY_SVCTYPE_LTE) {
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
  out.insert(std::make_pair("networkType", picojson::value(type_str)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult GetIps(connection_profile_h profile_handle, std::string* ip_addr_str,
                   std::string* ipv6_addr_str){
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

PlatformResult SysteminfoUtils::ReportWifiNetwork(picojson::object& out) {
  bool result_status = false;
  std::string result_ssid;
  std::string result_ip_address;
  std::string result_ipv6_address;
  std::string result_mac_address;
  double result_signal_strength = 0;

  connection_h connection_handle = nullptr;
  connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
  connection_profile_h profile_handle = nullptr;

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

  char* mac = NULL;
  error = wifi_get_mac_address(&mac);
  if(WIFI_ERROR_NONE == error) {
    LoggerD("macAddress fetched: %s", mac);
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
  if (CONNECTION_TYPE_WIFI == connection_type) {
    result_status = true;
    //gathering profile
    error = connection_get_current_profile(connection_handle, &profile_handle);
    if (CONNECTION_ERROR_NONE != error) {
      std::string log_msg = "Cannot get connection profile: " + std::to_string(error);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
    std::unique_ptr
    <std::remove_pointer<connection_profile_h>::type, int(*)(connection_profile_h)>
    profile_handle_ptr(profile_handle, &connection_profile_destroy);
    // automatically release the memory

    //gathering ssid
    char* essid = nullptr;
    error = connection_profile_get_wifi_essid(profile_handle, &essid);
    if (CONNECTION_ERROR_NONE == error) {
      result_ssid = essid;
      free(essid);
    }
    else {
      std::string log_msg = "Failed to get network ssid: " + std::to_string(error);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }

    //gathering ips
    PlatformResult ret = GetIps(profile_handle, &result_ip_address, &result_ipv6_address);
    if (ret.IsError()) {
      return ret;
    }

    //gathering strength
    int rssi = 0;
    error = connection_profile_get_wifi_rssi(profile_handle, &rssi);
    if (CONNECTION_ERROR_NONE == error) {
      result_signal_strength = (double) rssi/kWifiSignalStrengthDivideValue;
    }
    else {
      std::string log_msg = "Failed to get signal strength: " + std::to_string(error);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
  }
  else {
    LoggerD("Connection type = %d. WIFI is disabled", connection_type);
  }

  out.insert(std::make_pair("status", picojson::value(result_status ? kWifiStatusOn : kWifiStatusOff)));
  out.insert(std::make_pair("ssid", picojson::value(result_ssid)));
  out.insert(std::make_pair("ipAddress", picojson::value(result_ip_address)));
  out.insert(std::make_pair("ipv6Address", picojson::value(result_ipv6_address)));
  out.insert(std::make_pair("macAddress", picojson::value(result_mac_address)));
  out.insert(std::make_pair("signalStrength", picojson::value(std::to_string(result_signal_strength))));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult FetchVconfSettings(
    unsigned short *result_mcc,
    unsigned short *result_mnc,
    unsigned short *result_cell_id,
    unsigned short *result_lac,
    bool *result_is_roaming,
    bool *result_is_flight_mode)
{
  LoggerD("Entered");
  int result;
  if (0 != vconf_get_int(VCONFKEY_TELEPHONY_PLMN, &result)) {
    LoggerE("Cannot get mcc value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get mcc value");
  }
  *result_mcc = static_cast<unsigned short>(result) / kMccDivider;
  *result_mnc = static_cast<unsigned short>(result) % kMccDivider;

  if (0 != vconf_get_int(VCONFKEY_TELEPHONY_CELLID, &result)) {
    LoggerE("Cannot get cell_id value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get cell_id value");
  }
  *result_cell_id = static_cast<unsigned short>(result);

  if (0 != vconf_get_int(VCONFKEY_TELEPHONY_LAC, &result)) {
    LoggerE("Cannot get lac value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get lac value");
  }
  *result_lac = static_cast<unsigned short>(result);

  if (0 != vconf_get_int(VCONFKEY_TELEPHONY_SVC_ROAM, &result)) {
    LoggerE("Cannot get is_roaming value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get is_roaming value");
  }
  *result_is_roaming = (0 != result) ? true : false;

  if (0 != vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &result)) {
    LoggerE("Cannot get is_flight_mode value");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get is_flight_mode value");
  }
  *result_is_flight_mode = (0 != result) ? true : false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult FetchConnection(TapiHandle *tapi_handle, std::string* result_status,
                            std::string* result_apn, std::string* result_ip_address,
                            std::string* result_ipv6_address, std::string* result_imei)
{
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

    PlatformResult ret = GetIps(profile_handle, result_ip_address, result_ipv6_address);
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

PlatformResult SysteminfoUtils::ReportCellularNetwork(picojson::object& out) {
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
  PlatformResult ret = FetchVconfSettings(&result_mcc, &result_mnc, &result_cell_id, &result_lac,
                     &result_is_roaming, &result_is_flight_mode);
  if (ret.IsError()) {
    return ret;
  }
  //gathering connection informations
  ret = FetchConnection(system_info_listeners.GetTapiHandle(),
                  &result_status, &result_apn, &result_ip_address, &result_ipv6_address, &result_imei);
  if (ret.IsError()) {
    return ret;
  }

  out.insert(std::make_pair("status", picojson::value(result_status)));
  out.insert(std::make_pair("apn", picojson::value(result_apn)));
  out.insert(std::make_pair("ipAddress", picojson::value(result_ip_address)));
  out.insert(std::make_pair("ipv6Address", picojson::value(result_ipv6_address)));
  out.insert(std::make_pair("mcc", picojson::value(std::to_string(result_mcc))));
  out.insert(std::make_pair("mnc", picojson::value(std::to_string(result_mnc))));
  out.insert(std::make_pair("cellId", picojson::value(std::to_string(result_cell_id))));
  out.insert(std::make_pair("lac", picojson::value(std::to_string(result_lac))));
  out.insert(std::make_pair("isRoaming", picojson::value(result_is_roaming)));
  out.insert(std::make_pair("isFligthMode", picojson::value(result_is_flight_mode)));
  out.insert(std::make_pair("imei", picojson::value(result_imei)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SimCphsValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
  LoggerD("Entered");
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimCphsNetName_t *cphs_info = static_cast<TelSimCphsNetName_t*>(data);

  std::string result_operator;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    std::stringstream s;
    s << cphs_info->full_name;
    if (s.str().empty()) {
      s << cphs_info->short_name;
    }
    result_operator = s.str();
  } else {
    LoggerW("Failed to retrieve cphs_info: %d", access_rt);
  }
  sim_mgr.set_operator_name(result_operator);
  sim_mgr.TryReturn();
}

void SimMsisdnValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
  LoggerD("Entered");
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimMsisdnList_t *msisdn_info = static_cast<TelSimMsisdnList_t*>(data);

  std::string result_msisdn;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    if (msisdn_info->count > 0) {
      if (nullptr != msisdn_info->list[0].num) {
        result_msisdn = msisdn_info->list[0].num;
      } else {
        LoggerW("MSISDN number empty");
      }
    } else {
      LoggerW("msisdn_info list empty");
    }
  } else {
    LoggerW("Failed to retrieve msisdn_: %d", access_rt);
  }

  sim_mgr.set_msisdn(result_msisdn);
  sim_mgr.TryReturn();
}

void SimSpnValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
  LoggerD("Entered");
  TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
  TelSimSpn_t *spn_info = static_cast<TelSimSpn_t*>(data);

  std::string result_spn;
  if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
    result_spn = (char *)spn_info->spn;
  } else {
    LoggerW("Failed to retrieve spn_: %d", access_rt);
  }

  sim_mgr.set_spn(result_spn);
  sim_mgr.TryReturn();
}

PlatformResult SysteminfoUtils::ReportSim(picojson::object& out, unsigned long count) {
  return sim_mgr.GatherSimInformation(
      system_info_listeners.GetTapiHandles()[count], &out);
}

PlatformResult SysteminfoUtils::ReportPeripheral(picojson::object& out) {

  int wireless_display_status = 0;
  PlatformResult ret = GetVconfInt(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, wireless_display_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_MIRACAST_WFD_SOURCE_ON == wireless_display_status) {
      out.insert(std::make_pair(kVideoOutputString, picojson::value(true)));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  int hdmi_status = 0;
  ret = GetVconfInt(VCONFKEY_SYSMAN_HDMI, hdmi_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_SYSMAN_HDMI_CONNECTED == hdmi_status) {
      out.insert(std::make_pair(kVideoOutputString, picojson::value(true)));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  // TODO(r.galka) temporarily removed - not supported by platform
  //int popsync_status = 0;
  //ret = GetVconfInt(VCONFKEY_POPSYNC_ACTIVATED_KEY, popsync_status);
  //if (ret.IsSuccess()) {
  //  if (1 == popsync_status) {
  //    out.insert(std::make_pair(kVideoOutputString, picojson::value(true)));
  //    return PlatformResult(ErrorCode::NO_ERROR);
  //  }
  //}

  out.insert(std::make_pair(kVideoOutputString, picojson::value(false)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportMemory(picojson::object& out) {
  std::string state = MEMORY_STATE_NORMAL;
  int status = 0;
  PlatformResult ret = GetVconfInt(VCONFKEY_SYSMAN_LOW_MEMORY, status);
  if (ret.IsSuccess()) {
    switch (status) {
      case VCONFKEY_SYSMAN_LOW_MEMORY_SOFT_WARNING:
      case VCONFKEY_SYSMAN_LOW_MEMORY_HARD_WARNING:
        state = MEMORY_STATE_WARNING;
        break;
      case VCONFKEY_SYSMAN_LOW_MEMORY_NORMAL:
      default:
        state = MEMORY_STATE_NORMAL;
    }
  }

  out.insert(std::make_pair("state", picojson::value(state)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void CreateStorageInfo(const std::string& type, struct statfs& fs, picojson::object* out) {
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

PlatformResult SysteminfoUtils::ReportStorage(picojson::object& out) {
  int sdcardState = 0;
  struct statfs fs;

  picojson::value result = picojson::value(picojson::array());

  picojson::array& array = result.get<picojson::array>();
  array.push_back(picojson::value(picojson::object()));
  picojson::object& internal_obj = array.back().get<picojson::object>();

  if (statfs(kStorageInternalPath.c_str(), &fs) < 0) {
    LoggerE("There are no storage units detected");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "There are no storage units detected");
  }
  CreateStorageInfo(kTypeInternal, fs, &internal_obj);
  system_info_listeners.SetAvailableCapacityInternal(fs.f_bavail);

  if (0 == vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcardState)) {
    if (VCONFKEY_SYSMAN_MMC_MOUNTED == sdcardState){
      if (statfs(kStorageSdcardPath.c_str(), &fs) < 0) {
        LoggerE("MMC mounted, but not accessible");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "MMC mounted, but not accessible");
      }
      array.push_back(picojson::value(picojson::object()));
      picojson::object& external_obj = array.back().get<picojson::object>();
      CreateStorageInfo(kTypeMmc, fs, &external_obj);
      system_info_listeners.SetAvailableCapacityMmc(fs.f_bavail);
    }
  }

  out.insert(std::make_pair("storages", picojson::value(result)));
  return PlatformResult(ErrorCode::NO_ERROR);
}
PlatformResult SysteminfoUtils::ReportCameraFlash(picojson::object& out) {
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::RegisterBatteryListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterBatteryListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterBatteryListener()
{
  return system_info_listeners.UnregisterBatteryListener();
}


PlatformResult SysteminfoUtils::RegisterCpuListener(const SysteminfoUtilsCallback& callback,
                                                    SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCpuListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCpuListener()
{
  return system_info_listeners.UnregisterCpuListener();
}


PlatformResult SysteminfoUtils::RegisterStorageListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterStorageListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterStorageListener()
{
  return system_info_listeners.UnregisterStorageListener();
}

PlatformResult SysteminfoUtils::RegisterDisplayListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterDisplayListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterDisplayListener()
{
  return system_info_listeners.UnregisterDisplayListener();
}

PlatformResult SysteminfoUtils::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback,
                                                                  SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterDeviceOrientationListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterDeviceOrientationListener()
{
  return system_info_listeners.UnregisterDeviceOrientationListener();
}

PlatformResult SysteminfoUtils::RegisterLocaleListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterLocaleListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterLocaleListener()
{
  return system_info_listeners.UnregisterLocaleListener();
}

PlatformResult SysteminfoUtils::RegisterNetworkListener(const SysteminfoUtilsCallback& callback,
                                                        SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterNetworkListener()
{
  return system_info_listeners.UnregisterNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback,
                                                            SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterWifiNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterWifiNetworkListener()
{
  return system_info_listeners.UnregisterWifiNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback,
                                                                SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCellularNetworkListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCellularNetworkListener()
{
  return system_info_listeners.UnregisterCellularNetworkListener();
}

PlatformResult SysteminfoUtils::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback,
                                                           SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterPeripheralListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterPeripheralListener()
{
  return system_info_listeners.UnregisterPeripheralListener();
}

PlatformResult SysteminfoUtils::RegisterMemoryListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterMemoryListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterMemoryListener()
{
  return system_info_listeners.UnregisterMemoryListener();
}

PlatformResult SysteminfoUtils::RegisterCameraFlashListener(const SysteminfoUtilsCallback& callback,
                                                       SysteminfoInstance& instance)
{
  return system_info_listeners.RegisterCameraFlashListener(callback, instance);
}

PlatformResult SysteminfoUtils::UnregisterCameraFlashListener()
{
  return system_info_listeners.UnregisterCameraFlashListener();
}


static PlatformResult CheckStringCapability(const std::string& key, std::string* value, bool* fetched)
{
  LoggerD("Entered CheckStringCapability");
  *fetched = false;
  if (kTizenFeatureOpenglesTextureFormat == key) {
    PlatformResult ret = SystemInfoDeviceCapability::GetOpenglesTextureFormat(value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (kTizenFeatureCoreApiVersion == key) {
    *value = "2.3";
  } else if (key == kTizenFeaturePlatfromCoreCpuArch) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatfomCoreCpuArch(value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (kTizenFeaturePlatfromCoreFpuArch == key) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatfomCoreFpuArch(value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (kTizenFeatureProfile == key) {
    PlatformResult ret = SystemInfoDeviceCapability::GetProfile(value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (kTizenFeaturePlatformNativeApiVersion == key) {
    PlatformResult ret = SystemInfoDeviceCapability::GetNativeAPIVersion(value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (kTizenFeaturePlatformVersionName == key) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatformVersionName(value);
    if (ret.IsError()) {
      return ret;
    }
  } else {
    PlatformResult ret = SystemInfoDeviceCapability::GetValueString(key.substr(strlen("http://")).c_str(), value);
    if (ret.IsError()){
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  *fetched = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckBoolCapability(const std::string& key, bool* bool_value, bool* fetched)
{
  LoggerD("Entered CheckBoolCapability");
  *fetched = false;
  if(kTizenFeatureBluetoothAlwaysOn == key) {
    *bool_value = SystemInfoDeviceCapability::IsBluetoothAlwaysOn();
    *fetched = true;
  } else if (kTizenFeatureScreen == key) {
    *bool_value = SystemInfoDeviceCapability::IsScreen();
    *fetched = true;
  } else if (kTizenFeaturePlatformNativeOspCompatible == key) {
    PlatformResult ret = SystemInfoDeviceCapability::IsNativeOspCompatible(bool_value);
    if (ret.IsError()) {
      return ret;
    }
    *fetched = true;
  } else {
    PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
        key.substr(strlen("http://")).c_str(), bool_value);
    if (ret.IsSuccess()) {
      *fetched = true;
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckIntCapability(const std::string& key, std::string* value,
                                         bool* fetched)
{
  LoggerD("Entered CheckIntCapability");
  int result = 0;
  if (key == kTizenFeatureCpuFrequency) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatformCoreCpuFrequency(&result);
    if (ret.IsError()) {
      *fetched = false;
      return ret;
    }
  } else {
    PlatformResult ret = SystemInfoDeviceCapability::GetValueInt(
        key.substr(strlen("http://")).c_str(), &result);
    if (ret.IsError()) {
      *fetched = false;
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  *value = std::to_string(result);
  *fetched = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

///////////////////////   SystemInfoDeviceCapability   //////////////////////////////////////
PlatformResult SystemInfoDeviceCapability::GetCapability(const std::string& key,
                                                          picojson::value& result)
{
  LoggerD("Entered");
  picojson::object& result_obj = result.get<picojson::object>();

  std::string value = "";
  std::string type = "";
  bool bool_value = false ;
  bool is_fetched = false;

  PlatformResult ret = CheckBoolCapability(key, &bool_value, &is_fetched);
  if (ret.IsError()) {
    return ret;
  }
  if (is_fetched) {
    type = "bool";
  } else {
    ret = CheckIntCapability(key, &value, &is_fetched);
    if (ret.IsError()) {
      return ret;
    }
    if (is_fetched) {
      type = "int";
    } else {
      ret = CheckStringCapability(key, &value, &is_fetched);
      if (ret.IsError()) {
        return ret;
      }
      if(is_fetched) {
        type = "string";
      }
    }
  }

  if (type == "bool") {
    result_obj.insert(std::make_pair("value", picojson::value(bool_value)));
  } else if (type == "string" || type == "int") {
    result_obj.insert(std::make_pair("value", picojson::value(value)));
  } else {
    LoggerD("Value for given key was not found");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Value for given key was not found");
  }
  result_obj.insert(std::make_pair("type", picojson::value(type)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::IsInputKeyboardLayout(bool* result) {
  std::string input_keyboard_layout = "";
  PlatformResult ret = GetValueString("tizen.org/feature/input.keyboard.layout",
                                      &input_keyboard_layout);
  if (ret.IsError()) {
    return ret;
  }
  bool input_keyboard = false;
  ret = GetValueBool("tizen.org/feature/input.keyboard", &input_keyboard);
  if (ret.IsError()) {
    return ret;
  }

  // according to SystemInfo-DeviceCapabilities-dependency-table
  // inputKeyboard   inputKeyboardLayout
  //  O               O                   Possible
  //  O               X                   Possible
  //  X               X                   Possible
  //  X               O                   Impossible

  *result = input_keyboard ? !(input_keyboard_layout.empty()) : false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetOpenglesTextureFormat(std::string* result) {
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/opengles", &bool_result);
  if (!bool_result) {
    // this exception is converted to "Undefined" value in JS layer
    std::string log_msg = "OpenGL-ES is not supported";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
  }
  std::string texture_format = "";

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.utc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    texture_format += kOpenglesTextureUtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.ptc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTexturePtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.etc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTextureEtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.3dc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTexture3dc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.atc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTextureAtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.pvrtc", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTexturePvrtc;
  }

  if (texture_format.empty()) {
    // this exception is converted to "Undefined" value in JS layer
    std::string log_msg = "Platform error while getting OpenGL-ES texture format";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }
  *result = texture_format;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetPlatfomCoreCpuArch(std::string* return_value) {
  std::string result;
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv6", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    result = kPlatformCoreArmv6;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv7", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreArmv7;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.x86", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreX86;
  }

  if (result.empty()) {
    LoggerE("Platform error while retrieving platformCoreCpuArch: result is empty");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "platformCoreCpuArch result is empty");
  }
  *return_value = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetPlatfomCoreFpuArch(std::string* return_value) {
  std::string result;
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse2", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    result = kPlatformCoreSse2;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse3", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreSse3;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.ssse3", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreSsse3;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv2", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreVfpv2;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv3", &bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreVfpv3;
  }
  if (result.empty()) {
    LoggerE("Platform error while retrieving platformCoreFpuArch: result is empty");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "platformCoreFpuArch result is empty");
  }
  *return_value = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetProfile(std::string* return_value) {
  std::string profile = "";
  PlatformResult ret = GetValueString("tizen.org/feature/profile", &profile);
  if (ret.IsError()) {
    return ret;
  }

  *return_value = kProfileFull;
  if ( kPlatformFull == profile ) {
    *return_value = kProfileFull;
  } else if ( kPlatformMobile == profile ) {
    *return_value = kProfileMobile;
  } else if ( kPlatformWearable == profile ) {
    *return_value = kProfileWearable;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

bool SystemInfoDeviceCapability::IsBluetoothAlwaysOn() {
#ifdef PROFILE_MOBILE_FULL
  return false;
#elif PROFILE_MOBILE
  return false;
#elif PROFILE_WEARABLE
  return false;
#elif PROFILE_TV
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsScreen()
{
  return true;
}


PlatformResult SystemInfoDeviceCapability::GetPlatformCoreCpuFrequency(int* return_value)
{
  LoggerD("Entered");

  std::string freq;
  std::ifstream cpuinfo_max_freq("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
  if (!cpuinfo_max_freq.is_open()) {
    LoggerE("Failed to get cpu frequency");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to open file");
  }

  getline(cpuinfo_max_freq, freq);
  cpuinfo_max_freq.close();

  LoggerD("cpu frequency : %s", freq.c_str());
  *return_value = std::stoi(freq) / 1000; // unit: MHz

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::IsNativeOspCompatible(bool* result)
{
  LoggerD("Enter");
#ifdef PROFILE_WEARABLE
  *result = false;
  return PlatformResult(ErrorCode::NO_ERROR);
#else
  return GetValueBool(kTizenFeaturePlatformNativeOspCompatible, result);
#endif
}

PlatformResult SystemInfoDeviceCapability::GetNativeAPIVersion(std::string* return_value)
{
  LoggerD("Enter");
#ifdef PROFILE_WEARABLE
  *return_value = "";
  return PlatformResult(ErrorCode::NO_ERROR);
#else
  return GetValueString(kTizenFeaturePlatformNativeApiVersion, return_value);
#endif
}

PlatformResult SystemInfoDeviceCapability::GetPlatformVersionName(std::string* result)
{
  LoggerD("Enter");

  //Because of lack of 'http://tizen.org/feature/platform.version.name'
  //key on platform we use 'http://tizen.org/system/platform.name'.
  return GetValueString("tizen.org/system/platform.name", result);
}

} // namespace systeminfo
} // namespace webapi
