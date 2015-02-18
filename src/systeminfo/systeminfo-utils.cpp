/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

#include <sstream>
#include <memory>
#include <mutex>

#include <runtime_info.h>
#include <system_info.h>
#include <system_info_internal.h>
#include <sys/statfs.h>

#include <vconf.h>
#include <net_connection.h>
#include <tapi_common.h>
#include <ITapiModem.h>
#include <ITapiSim.h>
#include <device.h>

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
}
using namespace common;

//Callback functions declarations
static void OnBatteryChangedCb(keynode_t* node, void* event_ptr);
static gboolean OnCpuChangedCb(gpointer event_ptr);
static gboolean OnStorageChangedCb(gpointer event_ptr);
static void OnMmcChangedCb(keynode_t* node, void* event_ptr);
static void OnDisplayChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr);
static void OnDeviceOrientationChangedCb();
static void OnLocaleChangedCb(runtime_info_key_e key, void* event_ptr);
static void OnNetworkChangedCb(connection_type_e type, void* event_ptr);
static void OnNetworkValueChangedCb(const char* ipv4_address,
                                    const char* ipv6_address, void* event_ptr);
static void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr);
static void OnPeripheralChangedCb(keynode_t* node, void* event_ptr);
static void OnMemoryChangedCb(keynode_t* node, void* event_ptr);

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
const char* kStorageInternalPath = "/opt/usr/media";
const char* kStorageSdcardPath = "/opt/storage/sdcard";
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
const char* kTizenFeatureAccount = "http://tizen.org/feature/account";
const char* kTizenFeatureArchive = "http://tizen.org/feature/archive";
const char* kTizenFeatureBadge = "http://tizen.org/feature/badge";
const char* kTizenFeatureBookmark = "http://tizen.org/feature/bookmark";
const char* kTizenFeatureCalendar = "http://tizen.org/feature/calendar";
const char* kTizenFeatureContact  = "http://tizen.org/feature/contact";
const char* kTizenFeatureContent  = "http://tizen.org/feature/content";
const char* kTizenFeatureDatacontrol  = "http://tizen.org/feature/datacontrol";
const char* kTizenFeatureDatasync  = "http://tizen.org/feature/datasync";
const char* kTizenFeatureDownload = "http://tizen.org/feature/download";
const char* kTizenFeatureExif = "http://tizen.org/feature/exif";
const char* kTizenFeatureSystemsetting  = "http://tizen.org/feature/systemsetting"; //tv: false
const char* kTizenFeatureSystemSettingHomeScreen  = "http://tizen.org/feature/systemsetting.home_screen"; //TODO mobile: true, wearable: true
const char* kTizenFeatureSystemSettingLockScreen  = "http://tizen.org/feature/systemsetting.lock_screen"; //TODO mobile: true, wearable: false
const char* kTizenFeatureSystemSettingIncomingCall  = "http://tizen.org/feature/systemsetting.incoming_call"; //TODO mobile:true, wearable: true
const char* kTizenFeatureSystemSettingNotificationEmail  = "http://tizen.org/feature/systemsetting.notification_email"; //TODO mobile:true, wearable: false
const char* kTizenFeatureWebsetting  = "http://tizen.org/feature/websetting";
const char* kTizenFeaturePower  = "http://tizen.org/feature/power";
const char* kTizenFeatureGamepad = "http://tizen.org/feature/gamepad";
const char* kTizenFeatureMessaging = "http://tizen.org/feature/messaging";
const char* kTizenFeatureEmail = "http://tizen.org/feature/email";
const char* kTizenFeatureNotification = "http://tizen.org/feature/notification";
/*Battery*/
const char* kTizenFeatureBattery = "http://tizen.org/feature/battery";
/*input feature*/
const char* kTizenFeatureInputKeyboardLayout = "http://tizen.org/feature/input.keyboard.layout";
/*Multi-point touch feature*/
const char* kTizenFeatureMultitouchCount = "http://tizen.org/feature/multi_point_touch.point_count";
/*Network feature*/
const char* kTizenFeatureBluetooth = "http://tizen.org/feature/network.bluetooth.health"; //TODO mobile: true, wearable/tv: false
const char* kTizenFeatureBluetoothAlwaysOn = "http://tizen.org/capability/network.bluetooth.always_on"; //TODO mobile/wearable: false, tv: true
const char* kTizenFeatureNfcCardEmulation = "http://tizen.org/feature/network.nfc.card_emulation";
const char* kTizenFeatureOpenglesTextureFormat = "http://tizen.org/feature/opengles.texture_format";
const char* kTizenFeatureCoreApiVersion = "http://tizen.org/feature/platform.core.api.version";
const char* kTizenFeaturePlatfromCoreCpuArch = "http://tizen.org/feature/platform.core.cpu.arch";
const char* kTizenFeaturePlatfromCoreFpuArch = "http://tizen.org/feature/platform.core.fpu.arch";
const char* kTizenFeatureNativeApiVersion = "http://tizen.org/feature/platform.native.api.version";
const char* kTizenSystemPlatformVersion = "http://tizen.org/feature/platform.version";
const char* kTizenSystemPlatformWebApiVersion = "http://tizen.org/feature/platform.web.api.version";
/*profile feature*/
const char* kTizenFeatureProfile = "http://tizen.org/feature/profile";
/*Screen feature*/
const char* kTizenFeatureScreen = "http://tizen.org/feature/screen";
const char* kTizenFeatureScreenBpp = "http://tizen.org/feature/screen.bpp";
const char* kTizenFeatureScreenDpi = "http://tizen.org/feature/screen.dpi";
const char* kTizenFeatureScreenHeight = "http://tizen.org/feature/screen.height";
const char* kTizenFeatureScreenSIZE_320_320 = "http://tizen.org/feature/screen.size.normal.320.320";
const char* kTizenFeatureScreenWidth = "http://tizen.org/feature/screen.width";
/*Sensor faeture*/
const char* kTizenFeaturePressure = "http://tizen.org/feature/sensor.pressure";
const char* kTizenFeatureUltraviolet = "http://tizen.org/feature/sensor.ultraviolet"; //TODO
const char* kTizenFeaturePedometer = "http://tizen.org/feature/sensor.pedometer"; //TODO true
const char* kTizenFeatureWristUp = "http://tizen.org/feature/sensor.wrist_up"; //TODO false
const char* kTizenFeatureHrm = "http://tizen.org/feature/sensor.heart_rate_monitor"; //TODO true
/*duid*/
const char* kTizenSystemDuid = "http://tizen.org/system/duid";
/*platform*/
const char* kTizenSystemPlatformName = "http://tizen.org/system/platform.name";
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
  void FetchSimSyncProps(TapiHandle *tapi_handle);
  void ReturnSimToJS();

 public:
  SimDetailsManager();

  void GatherSimInformation(TapiHandle* handle, picojson::object* out);
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

void SimDetailsManager::GatherSimInformation(TapiHandle* handle, picojson::object* out)
{
  std::lock_guard<std::mutex> first_lock_sim(sim_info_mutex_);
  ResetSimHolder(out);

  FetchSimState(handle);
  if (kSimStatusReady == state_) {
    FetchSimSyncProps(handle);
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
    return;
  }
  //if sim state is not READY return default values and don't wait for callbacks
  TryReturn();
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

void SimDetailsManager::FetchSimSyncProps(TapiHandle *tapi_handle)
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
    throw UnknownException("Failed to get sim imsi");
  }

  //TODO add code for iccid value fetching, when proper API would be ready
  iccid_ = "";
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
    sim_result_obj_->insert(std::make_pair("state", state_));
    sim_result_obj_->insert(std::make_pair("operatorName", operator_name_));
    sim_result_obj_->insert(std::make_pair("msisdn", msisdn_));
    sim_result_obj_->insert(std::make_pair("iccid", iccid_));
    sim_result_obj_->insert(std::make_pair("mcc", std::to_string(mcc_)));
    sim_result_obj_->insert(std::make_pair("mnc", std::to_string(mnc_)));
    sim_result_obj_->insert(std::make_pair("msin", msin_));
    sim_result_obj_->insert(std::make_pair("spn", spn_));
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

/////////////////////////// SystemInfoDeviceOrientation ////////////////////////////////
class SystemInfoDeviceOrientation;
class DeviceOrientationChangeListener;
typedef std::shared_ptr<SystemInfoDeviceOrientation> SystemInfoDeviceOrientationPtr;

class SystemInfoDeviceOrientation:
    public DBusOperationListener
{
 public:
  SystemInfoDeviceOrientation();
  virtual ~SystemInfoDeviceOrientation();

  std::string status() const;
  bool is_auto_rotation() const;

  void SetDeviceOrientationChangeListener();
  void UnsetDeviceOrientationChangeListener();

  virtual void OnDBusSignal(int value);
 private:
  void RegisterDBus();
  void UnregisterDBus();
  bool FetchIsAutoRotation();
  std::string FetchStatus();

  std::string status_;
  bool is_auto_rotation_;

  bool registered_;

  DBusOperation dbus_op_;
};

SystemInfoDeviceOrientation::SystemInfoDeviceOrientation() :
                    registered_(false),
                    dbus_op_("org.tizen.system.coord",
                             "/Org/Tizen/System/Coord/Rotation",
                             "org.tizen.system.coord.rotation")
{
  LoggerD("Entered");
  is_auto_rotation_ = FetchIsAutoRotation();
  status_ = FetchStatus();
}

SystemInfoDeviceOrientation::~SystemInfoDeviceOrientation()
{
  LoggerD("Entered");
  UnsetDeviceOrientationChangeListener();
}

std::string SystemInfoDeviceOrientation::status() const
{
  return status_;
}

bool SystemInfoDeviceOrientation::is_auto_rotation() const
{
  return is_auto_rotation_;
}

bool SystemInfoDeviceOrientation::FetchIsAutoRotation()
{
  LoggerD("Entered");
  int is_auto_rotation = 0;

  if ( 0 == vconf_get_bool(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, &is_auto_rotation)) {
    if (is_auto_rotation) {
      return true;
    }
  } else {
    LoggerE("VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
    throw UnknownException(
        "VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
  }
  return false;
}

std::string SystemInfoDeviceOrientation::FetchStatus()
{
  LoggerD("Entered");

  DBusOperationArguments args;
  args.AddArgumentString("lcddim");
  args.AddArgumentString("staycurstate");
  args.AddArgumentString("NULL");
  args.AddArgumentInt32(0);

  std::string status = kOrientationPortraitPrimary;
  int ret = dbus_op_.InvokeSyncGetInt("DegreePhysicalRotation", &args);

  switch (ret) {
    case 0:
      //empty for purpose - go to next case
    case 1: //rotation 0
      status = kOrientationPortraitPrimary;
      break;
    case 2: //rotation 90
      status = kOrientationLandscapePrimary;
      break;
    case 3: //rotation 180
      status = kOrientationPortraitSecondary;
      break;
    case 4: //rotation 270
      status = kOrientationLandscapeSecondary;
      break;
    default:
      LoggerD("Unknown rotation value");
      break;
  }

  return status;
}

void SystemInfoDeviceOrientation::SetDeviceOrientationChangeListener()
{
  LoggerD("Enter");
  if (registered_) {
    LoggerD("already registered");
  } else {
    RegisterDBus();
    registered_ = true;
  }
}

void SystemInfoDeviceOrientation::UnsetDeviceOrientationChangeListener()
{
  LoggerD("Enter");
  if (!registered_) {
    LoggerD("not registered");
  } else {
    UnregisterDBus();
    registered_ = false;
  }
}

void SystemInfoDeviceOrientation::RegisterDBus()
{
  LoggerD("Enter");

  int ret = 0;
  DBusOperationArguments args;
  args.AddArgumentInt32(1);

  ret = dbus_op_.InvokeSyncGetInt("StartRotation", &args);

  if (ret != 0) {
    LoggerE("Failed to start rotation broadcast");
    throw UnknownException("Failed to start rotation broadcast");
  }

  dbus_op_.RegisterSignalListener("ChangedPhysicalRotation", this);
  LoggerD("registerSignalListener: ChangedPhysicalRotation");
}

void SystemInfoDeviceOrientation::UnregisterDBus()
{
  LoggerD("Enter");

  int ret = 0;
  dbus_op_.UnregisterSignalListener("ChangedPhysicalRotation", this);
  LoggerD("unregisterSignalListener: ChangedPhysicalRotation");

  DBusOperationArguments args;
  args.AddArgumentInt32(0);

  ret = dbus_op_.InvokeSyncGetInt("StartRotation", &args);

  if (ret != 0) {
    LoggerE("Failed to stop rotation broadcast");
    throw UnknownException("Failed to stop rotation broadcast");
  }
}

void SystemInfoDeviceOrientation::OnDBusSignal(int value)
{
  LoggerD("value : %d", value);

  switch (value) {
    case 0:
    case 1: //rotation 0
      LoggerD("ORIENTATION_PORTRAIT_PRIMARY");
      break;
    case 2: //rotation 90
      LoggerD("ORIENTATION_LANDSCAPE_PRIMARY");
      break;
    case 3: //rotation 180
      LoggerD("ORIENTATION_PORTRAIT_SECONDARY");
      break;
    case 4: //rotation 270
      LoggerD("ORIENTATION_LANDSCAPE_SECONDARY");
      break;
    default:
      LoggerD("Unknown rotation value");
      break;
  }

  LoggerD("call OnDeviceOrientationChangedCb");
  OnDeviceOrientationChangedCb();
}

/////////////////////////// SystemInfoListeners ////////////////////////////////

class SystemInfoListeners {
 public:
  SystemInfoListeners();
  ~SystemInfoListeners();

  void RegisterBatteryListener(const SysteminfoUtilsCallback& callback);
  void UnregisterBatteryListener();
  void RegisterCpuListener(const SysteminfoUtilsCallback& callback);
  void UnregisterCpuListener();
  void RegisterStorageListener(const SysteminfoUtilsCallback& callback);
  void UnregisterStorageListener();
  void RegisterDisplayListener(const SysteminfoUtilsCallback& callback);
  void UnregisterDisplayListener();
  void RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback);
  void UnregisterDeviceOrientationListener();
  void RegisterLocaleListener(const SysteminfoUtilsCallback& callback);
  void UnregisterLocaleListener();
  void RegisterNetworkListener(const SysteminfoUtilsCallback& callback);
  void UnregisterNetworkListener();
  void RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback);
  void UnregisterWifiNetworkListener();
  void RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback);
  void UnregisterCellularNetworkListener();
  void RegisterPeripheralListener(const SysteminfoUtilsCallback& callback);
  void UnregisterPeripheralListener();
  void RegisterMemoryListener(const SysteminfoUtilsCallback& callback);
  void UnregisterMemoryListener();

  void SetCpuInfoLoad(double load);
  void SetAvailableCapacityInternal(unsigned long long capacity);
  void SetAvailableCapacityMmc(unsigned long long capacity);

  void OnBatteryChangedCallback(keynode_t* node, void* event_ptr);
  void OnCpuChangedCallback(void* event_ptr);
  void OnStorageChangedCallback(void* event_ptr);
  void OnMmcChangedCallback(keynode_t* node, void* event_ptr);
  void OnDisplayChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceAutoRotationChangedCallback(keynode_t* node, void* event_ptr);
  void OnDeviceOrientationChangedCallback();
  void OnLocaleChangedCallback(runtime_info_key_e key, void* event_ptr);
  void OnNetworkChangedCallback(connection_type_e type, void* event_ptr);
  void OnNetworkValueCallback(const char* ipv4_address,
                              const char* ipv6_address, void* event_ptr);
  void OnCellularNetworkValueCallback(keynode_t *node, void *event_ptr);
  void OnPeripheralChangedCallback(keynode_t* node, void* event_ptr);
  void OnMemoryChangedCallback(keynode_t* node, void* event_ptr);

  TapiHandle* GetTapiHandle();
  TapiHandle** GetTapiHandles();
  connection_h GetConnectionHandle();
 private:
  static void RegisterVconfCallback(const char *in_key, vconf_callback_fn cb);
  static void UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb);
  void RegisterIpChangeCallback();
  void UnregisterIpChangeCallback();
  void InitTapiHandles();

  SystemInfoDeviceOrientationPtr m_orientation;
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

  TapiHandle *m_tapi_handles[TAPI_HANDLE_MAX+1];
  //for ip change callback
  connection_h m_connection_handle;
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
            m_connection_handle(nullptr)
{
  LoggerD("Entered");
}

SystemInfoListeners::~SystemInfoListeners(){
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

  unsigned int i = 0;
  while(m_tapi_handles[i]) {
    tel_deinit(m_tapi_handles[i]);
    i++;
  }
  if (nullptr != m_connection_handle) {
    connection_destroy(m_connection_handle);
  }
}

void SystemInfoListeners::RegisterBatteryListener(const SysteminfoUtilsCallback& callback)
{
  LoggerD("Entered");
  if (nullptr == m_battery_listener) {
    RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb);
    RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb);
    LoggerD("Added callback for BATTERY");
    m_battery_listener = callback;
  }
}

void SystemInfoListeners::UnregisterBatteryListener()
{
  if (nullptr != m_battery_listener) {
    UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb);
    UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb);
    LoggerD("Removed callback for BATTERY");
    m_battery_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterCpuListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_cpu_listener) {
    m_cpu_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnCpuChangedCb, nullptr);
    LoggerD("Added callback for CPU");
    m_cpu_listener = callback;
  }
}

void SystemInfoListeners::UnregisterCpuListener()
{
  if (nullptr != m_cpu_listener) {
    g_source_remove(m_cpu_event_id);
    m_cpu_event_id = 0;
    LoggerD("Removed callback for CPU");
    m_cpu_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterStorageListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_storage_listener) {
    try {
      RegisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    m_storage_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnStorageChangedCb, nullptr);
    LoggerD("Added callback for STORAGE");
    m_storage_listener = callback;
  }
}

void SystemInfoListeners::UnregisterStorageListener()
{
  if (nullptr != m_storage_listener) {
    try {
      UnregisterVconfCallback(VCONFKEY_SYSMAN_MMC_STATUS, OnMmcChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    g_source_remove(m_storage_event_id);
    m_storage_event_id = 0;
    LoggerD("Removed callback for STORAGE");
    m_storage_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterDisplayListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_display_listener) {
    RegisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb);
    LoggerD("Added callback for DISPLAY");
    m_display_listener = callback;
  }
}

void SystemInfoListeners::UnregisterDisplayListener()
{

  if (nullptr != m_display_listener) {
    UnregisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb);
    LoggerD("Removed callback for DISPLAY");
    m_display_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_orientation) {
    m_orientation.reset(new SystemInfoDeviceOrientation());
  }

  if (nullptr == m_device_orientation_listener) {
    RegisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb);
    m_orientation->SetDeviceOrientationChangeListener();

    LoggerD("Added callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = callback;
  }
}

void SystemInfoListeners::UnregisterDeviceOrientationListener()
{
  if (nullptr != m_device_orientation_listener) {
    UnregisterVconfCallback(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, OnDeviceAutoRotationChangedCb);
    if (nullptr != m_orientation) {
      m_orientation->UnsetDeviceOrientationChangeListener();
      m_orientation.reset();
    }

    LoggerD("Removed callback for DEVICE_ORIENTATION");
    m_device_orientation_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterLocaleListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_locale_listener) {
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_set_changed_cb(RUNTIME_INFO_KEY_REGION,
                                    OnLocaleChangedCb, nullptr) ) {
      LoggerE("Country change callback registration failed");
      throw UnknownException("Country change callback registration failed");
    }
    if (RUNTIME_INFO_ERROR_NONE !=
        runtime_info_set_changed_cb(RUNTIME_INFO_KEY_LANGUAGE,
                                    OnLocaleChangedCb, nullptr) ) {
      LoggerE("Language change callback registration failed");
      throw UnknownException("Language change callback registration failed");
    }
    LoggerD("Added callback for LOCALE");
    m_locale_listener = callback;
  }
}

void SystemInfoListeners::UnregisterLocaleListener()
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
}

void SystemInfoListeners::RegisterNetworkListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_network_listener) {
    connection_set_type_changed_cb(GetConnectionHandle(), OnNetworkChangedCb, nullptr);
    LoggerD("Added callback for NETWORK");
    m_network_listener = callback;
  }
}

void SystemInfoListeners::UnregisterNetworkListener()
{
  if (nullptr != m_network_listener) {

    LoggerD("Removed callback for NETWORK");
    m_network_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_wifi_network_listener) {
    if (nullptr == m_cellular_network_listener){
      //register if there is no callback both for wifi and cellular
      RegisterIpChangeCallback();
    } else {
      LoggerD("No need to register ip listener on platform, already registered");
    }
    LoggerD("Added callback for WIFI_NETWORK");
    m_wifi_network_listener = callback;
  }
}

void SystemInfoListeners::UnregisterWifiNetworkListener()
{
  //unregister if is wifi callback, but no cellular callback
  if (nullptr != m_wifi_network_listener && nullptr == m_cellular_network_listener) {
    UnregisterIpChangeCallback();
    LoggerD("Removed callback for WIFI_NETWORK");
  } else {
    LoggerD("Removed callback for WIFI_NETWORK, but cellular listener still works");
  }
  m_wifi_network_listener = nullptr;
}

void SystemInfoListeners::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_cellular_network_listener) {
    if (nullptr == m_wifi_network_listener){
      //register if there is no callback both for wifi and cellular
      RegisterIpChangeCallback();
    } else {
      LoggerD("No need to register ip listener on platform, already registered");
    }
    RegisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                          OnCellularNetworkValueChangedCb);
    RegisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                          OnCellularNetworkValueChangedCb);
    RegisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                          OnCellularNetworkValueChangedCb);
    RegisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                          OnCellularNetworkValueChangedCb);
    LoggerD("Added callback for CELLULAR_NETWORK");
    m_cellular_network_listener = callback;
  }
}

void SystemInfoListeners::UnregisterCellularNetworkListener()
{
  if (nullptr != m_cellular_network_listener) {
    UnregisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                            OnCellularNetworkValueChangedCb);
    UnregisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                            OnCellularNetworkValueChangedCb);
    UnregisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                            OnCellularNetworkValueChangedCb);
    UnregisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                            OnCellularNetworkValueChangedCb);
    if (nullptr == m_wifi_network_listener) {
      //register if there is no callback both for wifi and cellular
      UnregisterIpChangeCallback();
      LoggerD("Removed callback for CELLULAR_NETWORK");
    } else {
      LoggerD("Removed callback for CELLULAR_NETWORK, but cellular listener still works");
    }
  }
  m_cellular_network_listener = nullptr;
}

void SystemInfoListeners::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_peripheral_listener) {
    try {
      RegisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    try {
      RegisterVconfCallback(VCONFKEY_SYSMAN_HDMI, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    try {
      RegisterVconfCallback(VCONFKEY_POPSYNC_ACTIVATED_KEY, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    LoggerD("Added callback for PERIPHERAL");
    m_peripheral_listener = callback;
  }
}

void SystemInfoListeners::UnregisterPeripheralListener()
{
  if (nullptr != m_peripheral_listener) {
    try {
      UnregisterVconfCallback(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    try {
      UnregisterVconfCallback(VCONFKEY_SYSMAN_HDMI, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    try {
      UnregisterVconfCallback(VCONFKEY_POPSYNC_ACTIVATED_KEY, OnPeripheralChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    LoggerD("Removed callback for PERIPHERAL");
    m_peripheral_listener = nullptr;
  }
}

void SystemInfoListeners::RegisterMemoryListener(const SysteminfoUtilsCallback& callback)
{
  if (nullptr == m_memory_listener) {
    try {
      int value = 0;
      if (-1 != vconf_get_int(VCONFKEY_SYSMAN_LOW_MEMORY, &value)) {
        RegisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb);
      }
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    LoggerD("Added callback for MEMORY");
    m_memory_listener = callback;
  }
}

void SystemInfoListeners::UnregisterMemoryListener()
{
  if (nullptr != m_memory_listener) {
    try {
      UnregisterVconfCallback(VCONFKEY_SYSMAN_LOW_MEMORY, OnMemoryChangedCb);
    } catch (const PlatformException& e) {
      // empty on purpose
    }
    LoggerD("Removed callback for MEMORY");
    m_memory_listener = nullptr;
  }
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

void SystemInfoListeners::OnBatteryChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  if (nullptr != m_battery_listener) {
    m_battery_listener();
  }
}

void SystemInfoListeners::OnCpuChangedCallback(void* /*event_ptr*/)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu, false, result);
  if (ret.IsSuccess()) {
    if (m_cpu_load == m_last_cpu_load) {
      return;
    }
    if (nullptr != m_cpu_listener) {
      m_last_cpu_load = m_cpu_load;
      m_cpu_listener();
    }
  }
}

void SystemInfoListeners::OnStorageChangedCallback(void* /*event_ptr*/)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
  if (ret.IsSuccess()) {
    if (m_available_capacity_internal == m_last_available_capacity_internal) {
      return;
    }

    if (nullptr != m_storage_listener) {
      m_last_available_capacity_internal = m_available_capacity_internal;
      m_storage_listener();
    }
  }
}

void SystemInfoListeners::OnMmcChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  LoggerD("");
  picojson::value result = picojson::value(picojson::object());
  PlatformResult ret = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage, false, result);
  if (ret.IsSuccess()) {
    if (m_available_capacity_mmc == m_last_available_capacity_mmc) {
      return;
    }
    if (nullptr != m_storage_listener) {
      m_last_available_capacity_mmc = m_available_capacity_mmc;
      m_storage_listener();
    }
  }
}


void SystemInfoListeners::OnDisplayChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  if (nullptr != m_display_listener) {
    m_display_listener();
  }
}

void SystemInfoListeners::OnDeviceAutoRotationChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  if (nullptr != m_device_orientation_listener) {
    m_device_orientation_listener();
  }
}

void SystemInfoListeners::OnDeviceOrientationChangedCallback()
{
  if (nullptr != m_device_orientation_listener) {
    m_device_orientation_listener();
  }
}

void SystemInfoListeners::OnLocaleChangedCallback(runtime_info_key_e /*key*/, void* /*event_ptr*/)
{
  if (nullptr != m_locale_listener) {
    m_locale_listener();
  }
}

void SystemInfoListeners::OnNetworkChangedCallback(connection_type_e /*type*/, void* /*event_ptr*/)
{
  if (nullptr != m_network_listener) {
    m_network_listener();
  }
}

void SystemInfoListeners::OnNetworkValueCallback(const char* /*ipv4_address*/,
                                                 const char* /*ipv6_address*/, void* /*event_ptr*/)
{
  if (nullptr != m_wifi_network_listener) {
    m_wifi_network_listener();
  }
  if (nullptr != m_cellular_network_listener) {
    m_cellular_network_listener();
  }
}

void SystemInfoListeners::OnCellularNetworkValueCallback(keynode_t */*node*/, void */*event_ptr*/)
{
  if (nullptr != m_cellular_network_listener) {
    m_cellular_network_listener();
  }
}

void SystemInfoListeners::OnPeripheralChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  if (nullptr != m_peripheral_listener) {
    m_peripheral_listener();
  }
}

void SystemInfoListeners::OnMemoryChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
  if (nullptr != m_memory_listener) {
    m_memory_listener();
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

connection_h SystemInfoListeners::GetConnectionHandle()
{
  if (nullptr == m_connection_handle) {
    int error = connection_create(&m_connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
      LoggerE("Failed to create connection: %d", error);
      throw UnknownException("Cannot create connection");
    }
  }
  return m_connection_handle;
}

//////////////// Private ////////////////////

void SystemInfoListeners::RegisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
  if (0 != vconf_notify_key_changed(in_key, cb, nullptr)) {
    LoggerE("Failed to register vconf callback: %s", in_key);
    throw UnknownException("Failed to register vconf callback");
  }
}

void SystemInfoListeners::UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
  if (0 != vconf_ignore_key_changed(in_key, cb)) {
    LoggerE("Failed to unregister vconf callback: %s", in_key);
    throw UnknownException("Failed to unregister vconf callback");
  }
}

void SystemInfoListeners::RegisterIpChangeCallback()
{
  LoggerD("Registering connection callback");
  connection_h handle = GetConnectionHandle();
  int error = connection_set_ip_address_changed_cb(handle,
                                                   OnNetworkValueChangedCb, nullptr);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to register ip change callback: %d", error);
    throw UnknownException("Cannot register ip change callback");
  }
}

void SystemInfoListeners::UnregisterIpChangeCallback()
{
  LoggerD("Unregistering connection callback");
  connection_h handle = GetConnectionHandle();
  int error = connection_unset_ip_address_changed_cb(handle);
  if (CONNECTION_ERROR_NONE != error) {
    LoggerE("Failed to unregister ip change callback: %d", error);
    throw UnknownException("Cannot unregister ip change callback");
  }
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

void OnDeviceOrientationChangedCb()
{
  LoggerD("");
  system_info_listeners.OnDeviceOrientationChangedCallback();
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

/////////////////////////// SysteminfoUtils ////////////////////////////////

PlatformResult SystemInfoDeviceCapability::GetValueBool(const char *key, bool& value) {
  int ret = system_info_get_platform_bool(key, &value);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    std::string log_msg = "Platform error while getting bool value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  LoggerD("value[%s]: %s", key, value ? "true" : "false");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueInt(const char *key, int& value) {
  int ret = system_info_get_platform_int(key, &value);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    std::string log_msg = "Platform error while getting int value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  LoggerD("value[%s]: %d", key, value);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueString(const char *key, std::string& str_value) {
  char* value = nullptr;

  int ret = system_info_get_platform_string(key, &value);
  if (SYSTEM_INFO_ERROR_NONE == ret) {
    if (value != nullptr) {
      str_value = value;
      free(value);
      value = nullptr;
    }
  } else {
    std::string log_msg = "Platform error while getting string value: ";
    log_msg += std::string(key) + " " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
  }

  LoggerD("value[%s]: %s", key, str_value.c_str());
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

static PlatformResult GetSystemValueString(system_info_key_e key, std::string& platform_string) {
  char* platform_c_string;
  if (SYSTEM_INFO_ERROR_NONE
      == system_info_get_value_string(key, &platform_c_string)) {
    if (platform_c_string) {
      LoggerD("Build platfrom string %s", platform_c_string);
      platform_string = platform_c_string;
      free(platform_c_string);
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  const char* error_msg = "Error when retrieving value from platform API";
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

long long SysteminfoUtils::GetTotalMemory()
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_total(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    throw UnknownException(log_msg.c_str());
  }

  return static_cast<long long>(value*MEMORY_TO_BYTE);
}

long long SysteminfoUtils::GetAvailableMemory()
{
  LoggerD("Entered");

  unsigned int value = 0;

  int ret = device_memory_get_available(&value);
  if (ret != DEVICE_ERROR_NONE) {
    std::string log_msg = "Failed to get total memory: " + std::to_string(ret);
    LoggerE("%s", log_msg.c_str());
    throw UnknownException(log_msg.c_str());
  }

  return static_cast<long long>(value*MEMORY_TO_BYTE);
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

  value = 0;
  ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &value);
  if (kVconfErrorNone != ret) {
    std::string log_msg =  "Platform error while getting battery charging: ";
    LoggerE("%s%d",log_msg.c_str(), ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, (log_msg + std::to_string(ret)));
  }
  out.insert(std::make_pair("level", static_cast<double>(value)/kRemainingBatteryChargeMax));
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
  out.insert(std::make_pair("load", load / 100.0));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportDisplay(picojson::object& out) {
  int screenWidth = 0;
  int screenHeight = 0;
  unsigned long dotsPerInchWidth;
  unsigned long dotsPerInchHeight;
  int physicalWidth;
  int physicalHeight;
  double scaledBrightness;

  // FETCH RESOLUTION
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
      SYSTEM_INFO_KEY_SCREEN_WIDTH, &screenWidth)) {
    LoggerE("Cannot get value of screen width");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get value of screen width");
  }
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
      SYSTEM_INFO_KEY_SCREEN_HEIGHT, &screenHeight)) {
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
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
      SYSTEM_INFO_KEY_PHYSICAL_SCREEN_WIDTH, &physicalWidth)) {
    LoggerE("Cannot get value of phisical screen width");
    //TODO uncomment when api would support this key
    //return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get value of phisical screen width");
  }

  //FETCH PHYSICAL HEIGHT
  if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
      SYSTEM_INFO_KEY_PHYSICAL_SCREEN_HEIGHT, &physicalHeight)) {
    LoggerE("Cannot get value of phisical screen height");
    //TODO uncomment when api would support this key
    //return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get value of phisical screen height");
  }

  //FETCH BRIGHTNESS
  int brightness;
  if (kVconfErrorNone == vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness)) {
    scaledBrightness = static_cast<double>(brightness)/kDisplayBrightnessDivideValue;
  } else {
    LoggerE("Cannot get brightness value of display");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Cannot get brightness value of display");
  }

  out.insert(std::make_pair("resolutionWidth", std::to_string(screenWidth)));
  out.insert(std::make_pair("resolutionHeight", std::to_string(screenHeight)));
  out.insert(std::make_pair("dotsPerInchWidth", std::to_string(dotsPerInchWidth)));
  out.insert(std::make_pair("dotsPerInchHeight", std::to_string(dotsPerInchHeight)));
  out.insert(std::make_pair("physicalWidth", std::to_string(physicalWidth)));
  out.insert(std::make_pair("physicalHeight", std::to_string(physicalHeight)));
  out.insert(std::make_pair("brightness", scaledBrightness));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportDeviceOrientation(picojson::object& out) {
  SystemInfoDeviceOrientationPtr dev_orientation =
      SystemInfoDeviceOrientationPtr(new SystemInfoDeviceOrientation());
  std::string status = dev_orientation->status();
  bool auto_rotation_bool = dev_orientation->is_auto_rotation();
  out.insert(std::make_pair("isAutoRotation", auto_rotation_bool));
  out.insert(std::make_pair("status", status));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportBuild(picojson::object& out) {
  std::string model = "";
  PlatformResult ret = SystemInfoDeviceCapability::GetValueString("tizen.org/system/model_name", model);
  if (ret.IsError()) {
    return ret;
  }
  std::string manufacturer = "";
  ret = GetSystemValueString(SYSTEM_INFO_KEY_MANUFACTURER, manufacturer);
  if (ret.IsError()) {
    return ret;
  }
  std::string buildVersion = "";
  ret = GetSystemValueString(SYSTEM_INFO_KEY_BUILD_STRING, buildVersion);
  if (ret.IsError()) {
    return ret;
  }

  out.insert(std::make_pair("model", model));
  out.insert(std::make_pair("manufacturer", manufacturer));
  out.insert(std::make_pair("buildVersion", buildVersion));
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

  out.insert(std::make_pair("language", str_language));
  out.insert(std::make_pair("country", str_country));
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
  out.insert(std::make_pair("networkType", type_str));
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

  out.insert(std::make_pair("status", result_status ? kWifiStatusOn : kWifiStatusOff));
  out.insert(std::make_pair("ssid", result_ssid));
  out.insert(std::make_pair("ipAddress", result_ip_address));
  out.insert(std::make_pair("ipv6Address", result_ipv6_address));
  out.insert(std::make_pair("macAddress", result_mac_address));
  out.insert(std::make_pair("signalStrength", std::to_string(result_signal_strength)));
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

  out.insert(std::make_pair("status", result_status));
  out.insert(std::make_pair("apn", result_apn));
  out.insert(std::make_pair("ipAddress", result_ip_address));
  out.insert(std::make_pair("ipv6Address", result_ipv6_address));
  out.insert(std::make_pair("mcc", std::to_string(result_mcc)));
  out.insert(std::make_pair("mnc", std::to_string(result_mnc)));
  out.insert(std::make_pair("cellId", std::to_string(result_cell_id)));
  out.insert(std::make_pair("lac", std::to_string(result_lac)));
  out.insert(std::make_pair("isRoaming", result_is_roaming));
  out.insert(std::make_pair("isFligthMode", result_is_flight_mode));
  out.insert(std::make_pair("imei", result_imei));
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

  sim_mgr.GatherSimInformation(system_info_listeners.GetTapiHandles()[count], &out);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SysteminfoUtils::ReportPeripheral(picojson::object& out) {

  int wireless_display_status = 0;
  PlatformResult ret = GetVconfInt(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS, wireless_display_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_MIRACAST_WFD_SOURCE_ON == wireless_display_status) {
      out.insert(std::make_pair(kVideoOutputString, true));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  int hdmi_status = 0;
  ret = GetVconfInt(VCONFKEY_SYSMAN_HDMI, hdmi_status);
  if (ret.IsSuccess()) {
    if (VCONFKEY_SYSMAN_HDMI_CONNECTED == hdmi_status) {
      out.insert(std::make_pair(kVideoOutputString, true));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  int popsync_status = 0;
  ret = GetVconfInt(VCONFKEY_POPSYNC_ACTIVATED_KEY, popsync_status);
  if (ret.IsSuccess()) {
    if (1 == popsync_status) {
      out.insert(std::make_pair(kVideoOutputString, true));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  out.insert(std::make_pair(kVideoOutputString, false));
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

  out.insert(std::make_pair("state", state));
  return PlatformResult(ErrorCode::NO_ERROR);
}

static void CreateStorageInfo(const std::string& type, struct statfs& fs, picojson::object* out) {
  out->insert(std::make_pair("type", type));
  out->insert(std::make_pair("capacity", std::to_string(
      static_cast<unsigned long long>(fs.f_bsize) *
      static_cast<unsigned long long>(fs.f_blocks))));
  out->insert(std::make_pair("availableCapacity", std::to_string(
      static_cast<unsigned long long>(fs.f_bsize) *
      static_cast<unsigned long long>(fs.f_bavail))));
  bool isRemovable = (type == kTypeInternal) ? false : true;
  out->insert(std::make_pair("isRemovable", isRemovable));
}

PlatformResult SysteminfoUtils::ReportStorage(picojson::object& out) {
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
  system_info_listeners.SetAvailableCapacityInternal(fs.f_bavail);

  if (0 == vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcardState)) {
    if (VCONFKEY_SYSMAN_MMC_MOUNTED == sdcardState){
      if (statfs(kStorageSdcardPath, &fs) < 0) {
        LoggerE("MMC mounted, but not accessible");
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "MMC mounted, but not accessible");
      }
      array.push_back(picojson::value(picojson::object()));
      picojson::object& external_obj = array.back().get<picojson::object>();
      CreateStorageInfo(kTypeMmc, fs, &external_obj);
      system_info_listeners.SetAvailableCapacityMmc(fs.f_bavail);
    }
  }

  out.insert(std::make_pair("storages", result));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SysteminfoUtils::RegisterBatteryListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterBatteryListener(callback);
}

void SysteminfoUtils::UnregisterBatteryListener()
{
  system_info_listeners.UnregisterBatteryListener();
}


void SysteminfoUtils::RegisterCpuListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterCpuListener(callback);
}

void SysteminfoUtils::UnregisterCpuListener()
{
  system_info_listeners.UnregisterCpuListener();
}


void SysteminfoUtils::RegisterStorageListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterStorageListener(callback);
}

void SysteminfoUtils::UnregisterStorageListener()
{
  system_info_listeners.UnregisterStorageListener();
}

void SysteminfoUtils::RegisterDisplayListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterDisplayListener(callback);
}

void SysteminfoUtils::UnregisterDisplayListener()
{
  system_info_listeners.UnregisterDisplayListener();
}

void SysteminfoUtils::RegisterDeviceOrientationListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterDeviceOrientationListener(callback);
}

void SysteminfoUtils::UnregisterDeviceOrientationListener()
{
  system_info_listeners.UnregisterDeviceOrientationListener();
}

void SysteminfoUtils::RegisterLocaleListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterLocaleListener(callback);
}

void SysteminfoUtils::UnregisterLocaleListener()
{
  system_info_listeners.UnregisterLocaleListener();
}

void SysteminfoUtils::RegisterNetworkListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterNetworkListener(callback);
}

void SysteminfoUtils::UnregisterNetworkListener()
{
  system_info_listeners.UnregisterNetworkListener();
}

void SysteminfoUtils::RegisterWifiNetworkListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterWifiNetworkListener(callback);
}

void SysteminfoUtils::UnregisterWifiNetworkListener()
{
  system_info_listeners.UnregisterWifiNetworkListener();
}

void SysteminfoUtils::RegisterCellularNetworkListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterCellularNetworkListener(callback);
}

void SysteminfoUtils::UnregisterCellularNetworkListener()
{
  system_info_listeners.UnregisterCellularNetworkListener();
}

void SysteminfoUtils::RegisterPeripheralListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterPeripheralListener(callback);
}

void SysteminfoUtils::UnregisterPeripheralListener()
{
  system_info_listeners.UnregisterPeripheralListener();
}

void SysteminfoUtils::RegisterMemoryListener(const SysteminfoUtilsCallback& callback)
{
  system_info_listeners.RegisterMemoryListener(callback);
}

void SysteminfoUtils::UnregisterMemoryListener()
{
  system_info_listeners.UnregisterMemoryListener();
}


static PlatformResult CheckStringCapability(const std::string& key, std::string* value, bool& fetched)
{
  LoggerD("Entered CheckStringCapability");
  if (key == kTizenFeatureOpenglesTextureFormat) {
    PlatformResult ret = SystemInfoDeviceCapability::GetOpenglesTextureFormat(*value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (key == kTizenFeatureCoreApiVersion) {
    *value = "2.3";
  } else if (key == kTizenFeaturePlatfromCoreCpuArch) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatfomCoreCpuArch(*value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (key == kTizenFeaturePlatfromCoreFpuArch) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatfomCoreFpuArch(*value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (key == kTizenFeatureProfile) {
    PlatformResult ret = SystemInfoDeviceCapability::GetProfile(*value);
    if (ret.IsError()) {
      return ret;
    }
  } else if (key == kTizenSystemDuid) {
    *value = SystemInfoDeviceCapability::GetDuid();
  } else {
    PlatformResult ret = SystemInfoDeviceCapability::GetValueString(key.substr(strlen("http://")).c_str(), *value);
    if (ret.IsError()){
      fetched = false;
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  fetched = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckBoolCapability(const std::string& key, bool* bool_value, bool& fetched)
{
  LoggerD("Entered CheckBoolCapability");
  fetched = false;
  if(key == kTizenFeatureAccount) {
    *bool_value = SystemInfoDeviceCapability::IsAccount();
    fetched = true;
  } else if(key == kTizenFeatureArchive) {
    *bool_value = SystemInfoDeviceCapability::IsArchive();
    fetched = true;
  } else if(key == kTizenFeatureBadge) {
    *bool_value = SystemInfoDeviceCapability::IsBadge();
    fetched = true;
  } else if(key == kTizenFeatureBookmark) {
    *bool_value = SystemInfoDeviceCapability::IsBookmark();
    fetched = true;
  } else if(key == kTizenFeatureCalendar) {
    *bool_value = SystemInfoDeviceCapability::IsCalendar();
    fetched = true;
  } else if(key == kTizenFeatureContact) {
    *bool_value = SystemInfoDeviceCapability::IsContact();
    fetched = true;
  } else if(key == kTizenFeatureContent) {
    *bool_value = SystemInfoDeviceCapability::IsContent();
    fetched = true;
  } else if(key == kTizenFeatureDatacontrol) {
    *bool_value = SystemInfoDeviceCapability::IsDataControl();
    fetched = true;
  } else if(key == kTizenFeatureDatasync) {
    *bool_value = SystemInfoDeviceCapability::IsDataSync();
    fetched = true;
  } else if(key == kTizenFeatureDownload) {
    *bool_value = SystemInfoDeviceCapability::IsDownload();
    fetched = true;
  } else if(key == kTizenFeatureExif) {
    *bool_value = SystemInfoDeviceCapability::IsExif();
    fetched = true;
  } else if(key == kTizenFeatureSystemsetting) {
    *bool_value = SystemInfoDeviceCapability::IsSystemSetting();
    fetched = true;
  } else if(key == kTizenFeatureSystemSettingHomeScreen) {
    *bool_value = SystemInfoDeviceCapability::IsSystemSettingHomeScreen();
    fetched = true;
  } else if(key == kTizenFeatureSystemSettingLockScreen) {
    *bool_value = SystemInfoDeviceCapability::IsSystemSettingLockScreen();
    fetched = true;
  } else if(key == kTizenFeatureSystemSettingIncomingCall) {
    *bool_value = SystemInfoDeviceCapability::IsSystemSettingIncomingCall();
    fetched = true;
  } else if(key == kTizenFeatureSystemSettingNotificationEmail) {
    *bool_value = SystemInfoDeviceCapability::IsSystemSettingNotificationEmail();
    fetched = true;
  } else if(key == kTizenFeatureWebsetting) {
    *bool_value = SystemInfoDeviceCapability::IsWebSetting();
    fetched = true;
  } else if(key == kTizenFeaturePower) {
    *bool_value = SystemInfoDeviceCapability::IsPower();
    fetched = true;
  } else if(key == kTizenFeatureGamepad) {
    *bool_value = SystemInfoDeviceCapability::IsGamePad();
    fetched = true;
  } else if(key == kTizenFeatureMessaging) {
    *bool_value = SystemInfoDeviceCapability::IsMessaging();
    fetched = true;
  } else if(key == kTizenFeatureEmail) {
    *bool_value = SystemInfoDeviceCapability::IsMessagingEmail();
    fetched = true;
  } else if(key == kTizenFeatureNotification) {
    *bool_value = SystemInfoDeviceCapability::IsNotification();
    fetched = true;
  } else if(key == kTizenFeatureBluetooth) {
    *bool_value = SystemInfoDeviceCapability::IsBluetootHealth();
    fetched = true;
  } else if(key == kTizenFeatureBluetoothAlwaysOn) {
    *bool_value = SystemInfoDeviceCapability::IsBluetoothAlwaysOn();
    fetched = true;
  } else if(key == kTizenFeatureNfcCardEmulation) {
    *bool_value = SystemInfoDeviceCapability::IsNfcEmulation();
    fetched = true;
  } else if(key == kTizenFeatureBattery) {
    *bool_value = SystemInfoDeviceCapability::IsBattery();
    fetched = true;
  } else if(key == kTizenFeaturePressure) {
    *bool_value = SystemInfoDeviceCapability::IsPressure();
    fetched = true;
  } else if(key == kTizenFeatureUltraviolet) {
    *bool_value = SystemInfoDeviceCapability::IsUltraviolet();
    fetched = true;
  } else if(key == kTizenFeaturePedometer) {
    *bool_value = SystemInfoDeviceCapability::IsPedometer();
    fetched = true;
  } else if(key == kTizenFeatureWristUp) {
    *bool_value = SystemInfoDeviceCapability::IsWristUp();
    fetched = true;
  } else if(key == kTizenFeatureHrm) {
    *bool_value = SystemInfoDeviceCapability::IsHrm();
    fetched = true;
  } else if (key == kTizenFeatureScreen) {
    *bool_value = SystemInfoDeviceCapability::IsScreen();
    fetched = true;
  } else if(key == kTizenFeatureScreenSIZE_320_320) {
    PlatformResult ret = SystemInfoDeviceCapability::IsScreenSize320_320(*bool_value);
    if (ret.IsError()) {
      return ret;
    }
    fetched = true;
  } else {
    PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
        key.substr(strlen("http://")).c_str(), *bool_value);
    if (ret.IsSuccess()) {
      fetched = true;
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckIntCapability(const std::string& key, std::string* value, bool& fetched)
{
  LoggerD("Entered CheckIntCapability");
  int result = 0;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueInt(
      key.substr(strlen("http://")).c_str(), result);
  if (ret.IsSuccess()) {
    *value = std::to_string(result);
    fetched = true;
    return PlatformResult(ErrorCode::NO_ERROR);
  }
  fetched = false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

///////////////////////   SystemInfoDeviceCapability   //////////////////////////////////////
PlatformResult SystemInfoDeviceCapability::GetCapability(const std::string& key,
                                                          picojson::value& result)
{
  picojson::object& result_obj = result.get<picojson::object>();

  std::string value = "";
  std::string type = "";
  bool bool_value = false ;
  bool fetched_string = false;
  bool fetched_int = false;
  bool fetched_bool = false;
  PlatformResult ret = CheckStringCapability(key, &value, fetched_string);
  if (ret.IsError()) {
    return ret;
  }
  ret = CheckIntCapability(key, &value, fetched_int);
  if (ret.IsError()) {
    return ret;
  }
  ret = CheckBoolCapability(key, &bool_value, fetched_bool);
  if (ret.IsError()) {
    return ret;
  }
  if (fetched_string) {
    type = "string";
  } else if (fetched_int) {
    type = "int";
  } else if(fetched_bool) {
    type = "bool";
  }

  if (type == "bool") {
    result_obj.insert(std::make_pair("value", bool_value));
  } else if (type == "string" || type == "int") {
    result_obj.insert(std::make_pair("value", value));
  } else {
    LoggerD("Value for given key was not found");
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Value for given key was not found");
  }
  result_obj.insert(std::make_pair("type", type));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::IsInputKeyboardLayout(bool& result) {
  std::string input_keyboard_layout = "";
  PlatformResult ret = GetValueString("tizen.org/feature/input.keyboard.layout",
                                      input_keyboard_layout);
  if (ret.IsError()) {
    return ret;
  }
  bool input_keyboard = false;
  ret = GetValueBool("tizen.org/feature/input.keyboard", input_keyboard);
  if (ret.IsError()) {
    return ret;
  }

  // according to SystemInfo-DeviceCapabilities-dependency-table
  // inputKeyboard   inputKeyboardLayout
  //  O               O                   Possible
  //  O               X                   Possible
  //  X               X                   Possible
  //  X               O                   Impossible

  result = input_keyboard ? !(input_keyboard_layout.empty()) : false;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetOpenglesTextureFormat(std::string& result) {
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/opengles", bool_result);
  if (!bool_result) {
    // this exception is converted to "Undefined" value in JS layer
    std::string log_msg = "OpenGL-ES is not supported";
    LoggerE("%s", log_msg.c_str());
    return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, log_msg);
  }
  std::string texture_format = "";

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.utc", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    texture_format += kOpenglesTextureUtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.ptc", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTexturePtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.etc", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTextureEtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.3dc", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTexture3dc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.atc", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!texture_format.empty()) {
      texture_format += kOpenglesTextureDelimiter;
    }
    texture_format += kOpenglesTextureAtc;
  }

  ret = GetValueBool("tizen.org/feature/opengles.texture_format.pvrtc", bool_result);
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
  result = texture_format;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetPlatfomCoreCpuArch(std::string& return_value) {
  std::string result;
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv6", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    result = kPlatformCoreArmv6;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv7", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreArmv7;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.cpu.arch.x86", bool_result);
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
  return_value = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetPlatfomCoreFpuArch(std::string& return_value) {
  std::string result;
  bool bool_result = false;
  PlatformResult ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse2", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    result = kPlatformCoreSse2;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse3", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreSse3;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.ssse3", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreSsse3;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv2", bool_result);
  if (ret.IsError()) {
    return ret;
  }
  if (bool_result) {
    if (!result.empty()) {
      result += kPlatformCoreDelimiter;
    }
    result += kPlatformCoreVfpv2;
  }

  ret = GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv3", bool_result);
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
  return_value = result;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetProfile(std::string& return_value) {
  std::string profile = "";
  PlatformResult ret = GetValueString("tizen.org/feature/profile", profile);
  if (ret.IsError()) {
    return ret;
  }

  return_value = kProfileFull;
  if ( kPlatformFull == profile ) {
    return_value = kProfileFull;
  } else if ( kPlatformMobile == profile ) {
    return_value = kProfileMobile;
  } else if ( kPlatformWearable == profile ) {
    return_value = kProfileWearable;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

//Implementation ported from devel/webapi/refactoring branch,
//all flags are hardcoded for Kiran on top of this file
std::string SystemInfoDeviceCapability::GenerateDuid()
{
  LoggerD("Entered");

  bool supported = false;
  std::string duid = "";
  char* device_string = nullptr;
  int ret = 0;

#ifdef FEATURE_OPTIONAL_TELEPHONY
  ret = system_info_get_platform_bool("tizen.org/feature/network.telephony", &supported);
  if (ret != SYSTEM_INFO_ERROR_NONE) {
    LoggerE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.telephony).");
    return duid;
  }
  LoggerD("telephony is %s", supported ? "supported." : "not supported.");

  if (supported) {
    int time_count = 0;
    int status = 0;
    TapiHandle* handle = nullptr;
    while (time_count < 10) { //Wait 10 second.
      if (handle == nullptr) {
        handle = tel_init(nullptr);
      }

      if (handle != nullptr) {
        ret = tel_check_modem_power_status(handle, &status);
        if (ret != TAPI_API_SUCCESS) {
          tel_deinit(handle);
          LoggerD("It is failed to get IMEI.");
          return duid;
        }

        if (status == 0) {
          break;
        }
      } else {
        return duid;
      }
      usleep(1000000);
      time_count++;
    }
    if (handle != nullptr) {
      if (status == 0) {
        device_string = tel_get_misc_me_imei_sync(handle);
        tel_deinit(handle);
      } else {
        tel_deinit(handle);
        LoggerD("Modem is not ready to get IMEI.");
        return duid;
      }

      LoggerD("telephony.- device_string : %s", device_string);
    } else {
      LoggerD("Modem handle is not ready.");
      return duid;
    }
  } else {
    LoggerD("It is failed to generate DUID from telephony information");
  }

#elif FEATURE_OPTIONAL_BT
  ret = system_info_get_platform_bool("tizen.org/feature/network.bluetooth", &supported);
  if (ret != SYSTEM_INFO_ERROR_NONE) {
    LoggerE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.bluetooth).");
    return duid;
  }
  LoggerD("bluetooth is %s", supported ? "supported." : "not supported.");

  if (supported) {
#ifdef ENABLE_KIRAN
    char* bt_address = nullptr;
    ret = bt_adapter_get_address(&bt_address);

    if (ret == BT_ERROR_NONE && bt_address != nullptr) {
      char* temp = bt_address;
      device_string = (char*)malloc(SEED_LENGTH);
      memset(device_string, 0, SEED_LENGTH);

      strcat(device_string, "BLU");
      for (int i = 0; i < 6; i++)
      {
        strncat(device_string, temp, 2);
        temp+=3;
      }
      free(bt_address);

      LoggerD("BT - device_string : %s", device_string);
    }
#else
    FILE* fp = nullptr;
    fp = fopen("/csa/bluetooth/.bd_addr", "r");

    char buffer[32] = {0,};
    char* temp = buffer;

    if (fp != nullptr) {
      while (nullptr != fgets(temp, sizeof(buffer), fp)) {
        int len = strlen(buffer);
        if (buffer[len-1] == '\n' || buffer[len-1] == '\r') {
          buffer[len-1] =='\0';
        }
        temp = buffer;
        temp += len-1;
      }

      char* tmp_char = buffer;
      while (*tmp_char = toupper(*tmp_char))
      {
        tmp_char++;
      }

      device_string = (char*)malloc(SEED_LENGTH);
      memset(device_string, 0, SEED_LENGTH);

      strcat(device_string, "BLU");
      strcat(device_string, buffer);

      LoggerD("BT - device_string : %s", device_string);
    } else {
      LoggerD("fail file open.");
    }
#endif
  } else {
    LoggerD("It is failed to generate DUID from bluetooth information");
  }

#elif FEATURE_OPTIONAL_WI_FI
  ret = system_info_get_platform_bool("tizen.org/feature/network.wifi", &supported);
  if (ret != SYSTEM_INFO_ERROR_NONE) {
    LoggerE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.wifi).");
    return duid;
  }
  LoggerD("Wi-Fi is %s", supported ? "supported." : "not supported.");

  if (supported) {
    char* wifi_mac_address = nullptr;
    ret = wifi_get_mac_address(&wifi_mac_address);

    if (ret == WIFI_ERROR_NONE && wifi_mac_address != nullptr) {
      char* temp = wifi_mac_address;

      device_string = (char*)malloc(SEED_LENGTH);
      memset(device_string, 0, SEED_LENGTH);

      strcat(device_string, "WIF");

      for (int i = 0; i < 6; i++) {
        strncat(device_string, temp, 2);
        temp+=3;
      }
      free(wifi_mac_address);

      LoggerD("wifi - device_string : %s", pDeviceString);
    } else {
      LoggerD("It is failed to get mac address");
    }
  } else {
    LoggerD("It is failed to generate DUID from wi-fi information");
  }
#else
  LoggerD("It is failed to generate DUID");
#endif

  if (device_string != nullptr) {
    duid = GenerateId(device_string);
    free(device_string);
  }
  device_string = nullptr;

  LoggerD("duid : %s", duid.c_str());
  return duid;
}

std::string SystemInfoDeviceCapability::GenerateId(char* pDeviceString)
{
  LoggerD("Entered");
  unsigned long long value = 0;
  byte result[8]  {0,};

  GenerateCrc64(pDeviceString, &value);

  result[7] = value         & 0xFF;
  result[6] = (value >> 8)  & 0xFF;
  result[5] = (value >> 16) & 0xFF;
  result[4] = (value >> 24) & 0xFF;
  result[3] = (value >> 32) & 0xFF;
  result[2] = (value >> 40) & 0xFF;
  result[1] = (value >> 48) & 0xFF;
  result[0] = (value >> 56) & 0xFF;

  std::string duid = Base32Encode(result);

  return duid;
}

void SystemInfoDeviceCapability::GenerateCrc64(char* device_string, unsigned long long int* value)
{
  LoggerD("Entered");

  byte first_crypt[SEED_LENGTH + 1] = {0,};

  //0xCAFEBABECAFEBABE
  const char key_value[CRYPT_KEY_SIZE] = {(char)0xCA, (char)0xFE, (char)0xBA, (char)0xBE, (char)0xCA, (char)0xFE, (char)0xBA, (char)0xBE};

  int crypt_key_count = 0;
  for (int i = 0; i < SEED_LENGTH; i++) {
    first_crypt[i] = (unsigned char) (((device_string)[i] ^ key_value[crypt_key_count]));
    crypt_key_count ++;
    if(crypt_key_count == CRYPT_KEY_SIZE) {
      crypt_key_count = 0;
    }
  }

  unsigned long long int crc64_table[] = { 0x0000000000000000ULL, 0x01B0000000000000ULL, 0x0360000000000000ULL, 0x02D0000000000000ULL,
      0x06C0000000000000ULL, 0x0770000000000000ULL, 0x05A0000000000000ULL, 0x0410000000000000ULL,
      0x0D80000000000000ULL, 0x0C30000000000000ULL, 0x0EE0000000000000ULL, 0x0F50000000000000ULL,
      0x0B40000000000000ULL, 0x0AF0000000000000ULL, 0x0820000000000000ULL, 0x0990000000000000ULL,
      0x1B00000000000000ULL, 0x1AB0000000000000ULL, 0x1860000000000000ULL, 0x19D0000000000000ULL,
      0x1DC0000000000000ULL, 0x1C70000000000000ULL, 0x1EA0000000000000ULL, 0x1F10000000000000ULL,
      0x1680000000000000ULL, 0x1730000000000000ULL, 0x15E0000000000000ULL, 0x1450000000000000ULL,
      0x1040000000000000ULL, 0x11F0000000000000ULL, 0x1320000000000000ULL, 0x1290000000000000ULL,
      0x3600000000000000ULL, 0x37B0000000000000ULL, 0x3560000000000000ULL, 0x34D0000000000000ULL,
      0x30C0000000000000ULL, 0x3170000000000000ULL, 0x33A0000000000000ULL, 0x3210000000000000ULL,
      0x3B80000000000000ULL, 0x3A30000000000000ULL, 0x38E0000000000000ULL, 0x3950000000000000ULL,
      0x3D40000000000000ULL, 0x3CF0000000000000ULL, 0x3E20000000000000ULL, 0x3F90000000000000ULL,
      0x2D00000000000000ULL, 0x2CB0000000000000ULL, 0x2E60000000000000ULL, 0x2FD0000000000000ULL,
      0x2BC0000000000000ULL, 0x2A70000000000000ULL, 0x28A0000000000000ULL, 0x2910000000000000ULL,
      0x2080000000000000ULL, 0x2130000000000000ULL, 0x23E0000000000000ULL, 0x2250000000000000ULL,
      0x2640000000000000ULL, 0x27F0000000000000ULL, 0x2520000000000000ULL, 0x2490000000000000ULL,
      0x6C00000000000000ULL, 0x6DB0000000000000ULL, 0x6F60000000000000ULL, 0x6ED0000000000000ULL,
      0x6AC0000000000000ULL, 0x6B70000000000000ULL, 0x69A0000000000000ULL, 0x6810000000000000ULL,
      0x6180000000000000ULL, 0x6030000000000000ULL, 0x62E0000000000000ULL, 0x6350000000000000ULL,
      0x6740000000000000ULL, 0x66F0000000000000ULL, 0x6420000000000000ULL, 0x6590000000000000ULL,
      0x7700000000000000ULL, 0x76B0000000000000ULL, 0x7460000000000000ULL, 0x75D0000000000000ULL,
      0x71C0000000000000ULL, 0x7070000000000000ULL, 0x72A0000000000000ULL, 0x7310000000000000ULL,
      0x7A80000000000000ULL, 0x7B30000000000000ULL, 0x79E0000000000000ULL, 0x7850000000000000ULL,
      0x7C40000000000000ULL, 0x7DF0000000000000ULL, 0x7F20000000000000ULL, 0x7E90000000000000ULL,
      0x5A00000000000000ULL, 0x5BB0000000000000ULL, 0x5960000000000000ULL, 0x58D0000000000000ULL,
      0x5CC0000000000000ULL, 0x5D70000000000000ULL, 0x5FA0000000000000ULL, 0x5E10000000000000ULL,
      0x5780000000000000ULL, 0x5630000000000000ULL, 0x54E0000000000000ULL, 0x5550000000000000ULL,
      0x5140000000000000ULL, 0x50F0000000000000ULL, 0x5220000000000000ULL, 0x5390000000000000ULL,
      0x4100000000000000ULL, 0x40B0000000000000ULL, 0x4260000000000000ULL, 0x43D0000000000000ULL,
      0x47C0000000000000ULL, 0x4670000000000000ULL, 0x44A0000000000000ULL, 0x4510000000000000ULL,
      0x4C80000000000000ULL, 0x4D30000000000000ULL, 0x4FE0000000000000ULL, 0x4E50000000000000ULL,
      0x4A40000000000000ULL, 0x4BF0000000000000ULL, 0x4920000000000000ULL, 0x4890000000000000ULL,
      0xD800000000000000ULL, 0xD9B0000000000000ULL, 0xDB60000000000000ULL, 0xDAD0000000000000ULL,
      0xDEC0000000000000ULL, 0xDF70000000000000ULL, 0xDDA0000000000000ULL, 0xDC10000000000000ULL,
      0xD580000000000000ULL, 0xD430000000000000ULL, 0xD6E0000000000000ULL, 0xD750000000000000ULL,
      0xD340000000000000ULL, 0xD2F0000000000000ULL, 0xD020000000000000ULL, 0xD190000000000000ULL,
      0xC300000000000000ULL, 0xC2B0000000000000ULL, 0xC060000000000000ULL, 0xC1D0000000000000ULL,
      0xC5C0000000000000ULL, 0xC470000000000000ULL, 0xC6A0000000000000ULL, 0xC710000000000000ULL,
      0xCE80000000000000ULL, 0xCF30000000000000ULL, 0xCDE0000000000000ULL, 0xCC50000000000000ULL,
      0xC840000000000000ULL, 0xC9F0000000000000ULL, 0xCB20000000000000ULL, 0xCA90000000000000ULL,
      0xEE00000000000000ULL, 0xEFB0000000000000ULL, 0xED60000000000000ULL, 0xECD0000000000000ULL,
      0xE8C0000000000000ULL, 0xE970000000000000ULL, 0xEBA0000000000000ULL, 0xEA10000000000000ULL,
      0xE380000000000000ULL, 0xE230000000000000ULL, 0xE0E0000000000000ULL, 0xE150000000000000ULL,
      0xE540000000000000ULL, 0xE4F0000000000000ULL, 0xE620000000000000ULL, 0xE790000000000000ULL,
      0xF500000000000000ULL, 0xF4B0000000000000ULL, 0xF660000000000000ULL, 0xF7D0000000000000ULL,
      0xF3C0000000000000ULL, 0xF270000000000000ULL, 0xF0A0000000000000ULL, 0xF110000000000000ULL,
      0xF880000000000000ULL, 0xF930000000000000ULL, 0xFBE0000000000000ULL, 0xFA50000000000000ULL,
      0xFE40000000000000ULL, 0xFFF0000000000000ULL, 0xFD20000000000000ULL, 0xFC90000000000000ULL,
      0xB400000000000000ULL, 0xB5B0000000000000ULL, 0xB760000000000000ULL, 0xB6D0000000000000ULL,
      0xB2C0000000000000ULL, 0xB370000000000000ULL, 0xB1A0000000000000ULL, 0xB010000000000000ULL,
      0xB980000000000000ULL, 0xB830000000000000ULL, 0xBAE0000000000000ULL, 0xBB50000000000000ULL,
      0xBF40000000000000ULL, 0xBEF0000000000000ULL, 0xBC20000000000000ULL, 0xBD90000000000000ULL,
      0xAF00000000000000ULL, 0xAEB0000000000000ULL, 0xAC60000000000000ULL, 0xADD0000000000000ULL,
      0xA9C0000000000000ULL, 0xA870000000000000ULL, 0xAAA0000000000000ULL, 0xAB10000000000000ULL,
      0xA280000000000000ULL, 0xA330000000000000ULL, 0xA1E0000000000000ULL, 0xA050000000000000ULL,
      0xA440000000000000ULL, 0xA5F0000000000000ULL, 0xA720000000000000ULL, 0xA690000000000000ULL,
      0x8200000000000000ULL, 0x83B0000000000000ULL, 0x8160000000000000ULL, 0x80D0000000000000ULL,
      0x84C0000000000000ULL, 0x8570000000000000ULL, 0x87A0000000000000ULL, 0x8610000000000000ULL,
      0x8F80000000000000ULL, 0x8E30000000000000ULL, 0x8CE0000000000000ULL, 0x8D50000000000000ULL,
      0x8940000000000000ULL, 0x88F0000000000000ULL, 0x8A20000000000000ULL, 0x8B90000000000000ULL,
      0x9900000000000000ULL, 0x98B0000000000000ULL, 0x9A60000000000000ULL, 0x9BD0000000000000ULL,
      0x9FC0000000000000ULL, 0x9E70000000000000ULL, 0x9CA0000000000000ULL, 0x9D10000000000000ULL,
      0x9480000000000000ULL, 0x9530000000000000ULL, 0x97E0000000000000ULL, 0x9650000000000000ULL,
      0x9240000000000000ULL, 0x93F0000000000000ULL, 0x9120000000000000ULL, 0x9090000000000000ULL
  };

  byte* pu8 = first_crypt;
  unsigned long long int* crc_table = nullptr;
  int input_size = SEED_LENGTH + 1;

  *value = 0ULL;

  crc_table = const_cast <unsigned long long int*> (crc64_table);

  while (input_size--) {
    *value = crc_table[(*value ^ *pu8++) & 0xff] ^ (*value >> 8);
  }

  LoggerD("value : %llu", value);
}

std::string SystemInfoDeviceCapability::Base32Encode(byte* value)
{
  LoggerD("Entered");

  byte* encoding_pointer = nullptr;
  byte* src_pointer = nullptr;
  int i = 0;

  const char base32_char_set[] = "abcdefghijklmnopqrstuvwxyz0123456789";

  int input_size = 8;

  byte* buffer = nullptr;
  int buffer_length = 0;

  int src_size = input_size+1;

  src_pointer = (byte*)malloc(sizeof(byte)*src_size);
  memset(src_pointer, 0, sizeof(byte)*src_size);
  memcpy(src_pointer, value, sizeof(byte)*input_size);

  buffer_length = 8 * (input_size / 5 + 1) + 1;
  buffer = (byte*) calloc(1, buffer_length);

  encoding_pointer = buffer;

  for (i = 0; i < input_size; i += 5) {
    encoding_pointer[0] = base32_char_set[(src_pointer[0] >> 3)];
    encoding_pointer[1] = base32_char_set[((src_pointer[0] & 0x07) << 2) | ((src_pointer[1] & 0xc0) >> 6)];
    encoding_pointer[2] = (i + 1 < input_size) ? base32_char_set[((src_pointer[1] & 0x3e) >> 1)] : '\0';
    encoding_pointer[3] = (i + 1 < input_size) ? base32_char_set[((src_pointer[1] & 0x01) << 4) | ((src_pointer[2] & 0xf0) >> 4)] : '\0';
    encoding_pointer[4] = (i + 2 < input_size) ? base32_char_set[((src_pointer[2] & 0x0f) << 1) | ((src_pointer[3] & 0x80) >> 7)] : '\0';
    encoding_pointer[5] = (i + 3 < input_size) ? base32_char_set[((src_pointer[3] & 0x3e) >> 2)] : '\0';
    encoding_pointer[6] = (i + 3 < input_size) ? base32_char_set[((src_pointer[3] & 0x03) << 3) | ((src_pointer[4] & 0xe0) >> 5)] : '\0';
    encoding_pointer[7] = (i + 4 < input_size) ? base32_char_set[((src_pointer[4] & 0x1f))] : '\0';

    src_pointer += 5;
    encoding_pointer += 8;
  }

  std::string duid((char*)(buffer));

  if (buffer != nullptr) {
    free(buffer);
  }

  return duid;
}

std::string SystemInfoDeviceCapability::GetDuid()
{
  return GenerateDuid();
}

//////additional capabilities
bool SystemInfoDeviceCapability::IsAccount()
{
#ifdef FEATURE_OPTIONAL_ACCOUNT
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsArchive()
{
#ifdef FEATURE_OPTIONAL_ARCHIVE
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsBadge()
{
#ifdef FEATURE_OPTIONAL_BADGE
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsBookmark()
{
#ifdef FEATURE_OPTIONAL_BOOKMARK
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsCalendar()
{
#ifdef FEATURE_OPTIONAL_CALENDAR
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsContact()
{
#ifdef FEATURE_OPTIONAL_CONTACT
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsContent()
{
#ifdef FEATURE_OPTIONAL_CONTENT
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsDataControl()
{
#ifdef FEATURE_OPTIONAL_DATACONTROL
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsDataSync()
{
#ifdef FEATURE_OPTIONAL_DATASYNC
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsDownload()
{
#ifdef FEATURE_OPTIONAL_DOWNLOAD
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsExif()
{
#ifdef FEATURE_OPTIONAL_EXIF
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsGamePad()
{
#ifdef FEATURE_OPTIONAL_GAME_PAD
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsMessagingEmail()
{
#ifdef FEATURE_OPTIONAL_MESSAGING_EMAIL
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsMessaging()
{
#ifdef FEATURE_OPTIONAL_MESSAGING
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsBluetootHealth()
{
#ifdef FEATURE_OPTIONAL_BT_HEALTH
  return true;
#else
  return false;
#endif
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

bool SystemInfoDeviceCapability::IsNfcEmulation()
{
#ifdef FEATURE_OPTIONAL_NFC_EMULATION
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsNotification()
{
#ifdef FEATURE_OPTIONAL_NOTIFICATION
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsPower()
{
#ifdef FEATURE_OPTIONAL_POWER
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsWebSetting()
{
#ifdef FEATURE_OPTIONAL_WEB_SETTING
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsSystemSetting()
{
#ifdef FEATURE_OPTIONAL_SYSTEM_SETTING
  return true;
#else
  return false;
#endif
}


bool SystemInfoDeviceCapability::IsSystemSettingHomeScreen() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#elif PROFILE_MOBILE
  return true;
#elif PROFILE_WEARABLE
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsSystemSettingLockScreen() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#elif PROFILE_MOBILE
  return true;
#elif PROFILE_WEARABLE
  return false;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsSystemSettingIncomingCall() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#elif PROFILE_MOBILE
  return true;
#elif PROFILE_WEARABLE
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsSystemSettingNotificationEmail() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#elif PROFILE_MOBILE
  return true;
#elif PROFILE_WEARABLE
  return false;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsBattery() {
#ifdef PROFILE_TV
  return false;
#else
  return true;
#endif
}

bool SystemInfoDeviceCapability::IsCoreAPI()
{
#ifdef FEATURE_OPTIONAL_CORE_API
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsPressure() {
  return false;
}

bool SystemInfoDeviceCapability::IsUltraviolet() {
  return false;
}

bool SystemInfoDeviceCapability::IsPedometer() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsWristUp() {
#ifdef PROFILE_MOBILE_FULL
  return false;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsHrm() {
#ifdef PROFILE_MOBILE_FULL
  return true;
#else
  return false;
#endif
}

bool SystemInfoDeviceCapability::IsScreen()
{
  return true;
}

PlatformResult SystemInfoDeviceCapability::IsScreenSize320_320(bool& return_value)
{
  int height = 0;
  PlatformResult ret = SystemInfoDeviceCapability::GetValueInt(
      "tizen.org/feature/screen.height", height);
  if (ret.IsError()) {
    return ret;
  }
  int width = 0;
  ret = SystemInfoDeviceCapability::GetValueInt(
      "tizen.org/feature/screen.width", width);
  if (ret.IsError()) {
    return ret;
  }
  if (height != 320 || width != 320) {
    return_value = false;
  } else {
    return_value = true;
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // namespace systeminfo
} // namespace webapi
