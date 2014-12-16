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
#include <ITapiSim_product.h>

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

    void ResetSimHolder(picojson::object* out);
    void FetchSimState(TapiHandle *tapi_handle);
    void FetchSimSyncProps(TapiHandle *tapi_handle);
    void ReturnSimToJS();

public:
    SimDetailsManager();

    void GatherSimInformation(TapiHandle* handle, picojson::object* out);
    void TryReturn();

    void set_operator_name(const std::string& name)
    {
        std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
        operator_name_ = name;
        --to_process_;
        LOGD("Operator name: %s", operator_name_.c_str());
    };
    void set_msisdn(const std::string& msisdn)
    {
        std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
        this->msisdn_ = msisdn;
        --to_process_;
        LOGD("MSISDN number: %s", this->msisdn_.c_str());
    };
    void set_spn(const std::string& spn)
    {
        std::lock_guard<std::mutex> lock(sim_to_process_mutex_);
        this->spn_ = spn;
        --to_process_;
        LOGD("SPN value: %s", this->spn_.c_str());
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
        to_process_(0)
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
                LOGE("Failed getting cphs netname: %d", result);
            }

            result = tel_get_sim_msisdn(handle, SimMsisdnValueCallback, nullptr);
            if (TAPI_API_SUCCESS == result) {
                ++to_process_;
            } else {
                LOGE("Failed getting msisdn: %d", result);
            }

            result = tel_get_sim_spn(handle, SimSpnValueCallback, nullptr);
            if (TAPI_API_SUCCESS == result) {
                ++to_process_;
            } else {
                LOGE("Failed getting spn: %d", result);
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
    LOGD("Entered");
    if (nullptr == tapi_handle) {
        LOGE("Tapi handle is null");
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
    LOGD("Entered");
    TelSimImsiInfo_t imsi;
    int error = tel_get_sim_imsi(tapi_handle, &imsi);
    if (TAPI_API_SUCCESS == error) {
        LOGD("mcc: %s, mnc: %s, msin: %s", imsi.szMcc, imsi.szMnc, imsi.szMsin);
        mcc_ = std::stoul(imsi.szMcc);
        mnc_ = std::stoul(imsi.szMnc);
        msin_ = imsi.szMsin;
    }
    else {
        LOGE("Failed to get sim imsi: %d", error);
        throw UnknownException("Failed to get sim imsi");
    }

    TelSimResponseData_t outparam;
    memset(&outparam, 0, sizeof (TelSimResponseData_t));
    error = tel_request_sim_sync(tapi_handle, TAPI_SIM_GET_ICCID, nullptr,
            &outparam);
    if (TAPI_API_SUCCESS == error) {
        iccid_ = outparam.iccid_info.icc_num;
        LOGD("ICCID : %s", iccid_.c_str());
    }
    else {
        LOGE("Failed to get iccid info: %d", error);
        throw UnknownException("Failed to get iccid_ info");
    }
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
    LOGD("Entered");
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
        LOGE("No sim returned JSON object pointer is null");
    }
}

void SimDetailsManager::TryReturn(){
    if (0 == to_process_){
        LOGD("Returning property to JS");
        ReturnSimToJS();
        sim_info_mutex_.unlock();
    } else {
        LOGD("Not ready yet - waiting");
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
    LOGD("Entered");
    is_auto_rotation_ = FetchIsAutoRotation();
    status_ = FetchStatus();
}

SystemInfoDeviceOrientation::~SystemInfoDeviceOrientation()
{
    LOGD("Entered");
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
    LOGD("Entered");
    int is_auto_rotation = 0;

    if ( 0 == vconf_get_bool(VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL, &is_auto_rotation)) {
        if (is_auto_rotation) {
            return true;
        }
    } else {
        LOGE("VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
        throw UnknownException(
                "VCONFKEY_SETAPPL_AUTO_ROTATE_SCREEN_BOOL check failed");
    }
    return false;
}

std::string SystemInfoDeviceOrientation::FetchStatus()
{
    LOGD("Entered");

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
        LOGD("Unknown rotation value");
        break;
    }

    return status;
}

void SystemInfoDeviceOrientation::SetDeviceOrientationChangeListener()
{
    LOGD("Enter");
    if (registered_) {
        LOGD("already registered");
    } else {
        RegisterDBus();
        registered_ = true;
    }
}

void SystemInfoDeviceOrientation::UnsetDeviceOrientationChangeListener()
{
    LOGD("Enter");
    if (!registered_) {
        LOGD("not registered");
    } else {
        UnregisterDBus();
        registered_ = false;
    }
}

void SystemInfoDeviceOrientation::RegisterDBus()
{
    LOGD("Enter");

    int ret = 0;
    DBusOperationArguments args;
    args.AddArgumentInt32(1);

    ret = dbus_op_.InvokeSyncGetInt("StartRotation", &args);

    if (ret != 0) {
        LOGE("Failed to start rotation broadcast");
        throw UnknownException("Failed to start rotation broadcast");
    }

    dbus_op_.RegisterSignalListener("ChangedPhysicalRotation", this);
    LOGD("registerSignalListener: ChangedPhysicalRotation");
}

void SystemInfoDeviceOrientation::UnregisterDBus()
{
    LOGD("Enter");

    int ret = 0;
    dbus_op_.UnregisterSignalListener("ChangedPhysicalRotation", this);
    LOGD("unregisterSignalListener: ChangedPhysicalRotation");

    DBusOperationArguments args;
    args.AddArgumentInt32(0);

    ret = dbus_op_.InvokeSyncGetInt("StartRotation", &args);

    if (ret != 0) {
        LOGE("Failed to stop rotation broadcast");
        throw UnknownException("Failed to stop rotation broadcast");
    }
}

void SystemInfoDeviceOrientation::OnDBusSignal(int value)
{
    LOGD("value : %d", value);

    switch (value) {
    case 0:
    case 1: //rotation 0
        LOGD("ORIENTATION_PORTRAIT_PRIMARY");
        break;
    case 2: //rotation 90
        LOGD("ORIENTATION_LANDSCAPE_PRIMARY");
        break;
    case 3: //rotation 180
        LOGD("ORIENTATION_PORTRAIT_SECONDARY");
        break;
    case 4: //rotation 270
        LOGD("ORIENTATION_LANDSCAPE_SECONDARY");
        break;
    default:
        LOGD("Unknown rotation value");
        break;
    }

    LOGD("call OnDeviceOrientationChangedCb");
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

    TapiHandle* GetTapiHandle();
    connection_h GetConnectionHandle();
private:
    static void RegisterVconfCallback(const char *in_key, vconf_callback_fn cb);
    static void UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb);
    void RegisterIpChangeCallback();
    void UnregisterIpChangeCallback();

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

    TapiHandle *m_tapi_handle;
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
        m_tapi_handle(nullptr),
        m_connection_handle(nullptr)
{
    LOGD("Entered");
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
    if (nullptr != m_tapi_handle) {
        tel_deinit(m_tapi_handle);
    }
    if (nullptr != m_connection_handle) {
        connection_destroy(m_connection_handle);
    }
 }

void SystemInfoListeners::RegisterBatteryListener(const SysteminfoUtilsCallback& callback)
{
    LOGD("Entered");
    if (nullptr == m_battery_listener) {
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb);
        RegisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb);
        LOGD("Added callback for BATTERY");
        m_battery_listener = callback;
    }
}

void SystemInfoListeners::UnregisterBatteryListener()
{
    if (nullptr != m_battery_listener) {
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CAPACITY, OnBatteryChangedCb);
        UnregisterVconfCallback(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, OnBatteryChangedCb);
        LOGD("Removed callback for BATTERY");
        m_battery_listener = nullptr;
    }
}

void SystemInfoListeners::RegisterCpuListener(const SysteminfoUtilsCallback& callback)
{
    if (nullptr == m_cpu_listener) {
        m_cpu_event_id = g_timeout_add_seconds(kPropertyWatcherTime, OnCpuChangedCb, nullptr);
        LOGD("Added callback for CPU");
        m_cpu_listener = callback;
    }
}

void SystemInfoListeners::UnregisterCpuListener()
{
    if (nullptr != m_cpu_listener) {
        g_source_remove(m_cpu_event_id);
        m_cpu_event_id = 0;
        LOGD("Removed callback for CPU");
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
        LOGD("Added callback for STORAGE");
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
        LOGD("Removed callback for STORAGE");
        m_storage_listener = nullptr;
     }
}

void SystemInfoListeners::RegisterDisplayListener(const SysteminfoUtilsCallback& callback)
{
    if (nullptr == m_display_listener) {
        RegisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb);
        LOGD("Added callback for DISPLAY");
        m_display_listener = callback;
    }
}

void SystemInfoListeners::UnregisterDisplayListener()
{

    if (nullptr != m_display_listener) {
        UnregisterVconfCallback(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, OnDisplayChangedCb);
        LOGD("Removed callback for DISPLAY");
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

        LOGD("Added callback for DEVICE_ORIENTATION");
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

        LOGD("Removed callback for DEVICE_ORIENTATION");
        m_device_orientation_listener = nullptr;
    }
}

void SystemInfoListeners::RegisterLocaleListener(const SysteminfoUtilsCallback& callback)
{
    if (nullptr == m_locale_listener) {
        if (RUNTIME_INFO_ERROR_NONE !=
                runtime_info_set_changed_cb(RUNTIME_INFO_KEY_REGION,
                        OnLocaleChangedCb, nullptr) ) {
            LOGE("Country change callback registration failed");
            throw UnknownException("Country change callback registration failed");
        }
        if (RUNTIME_INFO_ERROR_NONE !=
                runtime_info_set_changed_cb(RUNTIME_INFO_KEY_LANGUAGE,
                        OnLocaleChangedCb, nullptr) ) {
            LOGE("Language change callback registration failed");
            throw UnknownException("Language change callback registration failed");
        }
        LOGD("Added callback for LOCALE");
        m_locale_listener = callback;
    }
}

void SystemInfoListeners::UnregisterLocaleListener()
{
    if (nullptr != m_locale_listener) {
        if (RUNTIME_INFO_ERROR_NONE !=
                runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_LANGUAGE) ) {
            LOGE("Unregistration of language change callback failed");
        }
        if (RUNTIME_INFO_ERROR_NONE !=
                runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_REGION) ) {
            LOGE("Unregistration of country change callback failed");
        }
        LOGD("Removed callback for LOCALE");
        m_locale_listener = nullptr;
    }
}

void SystemInfoListeners::RegisterNetworkListener(const SysteminfoUtilsCallback& callback)
{
    if (nullptr == m_network_listener) {
        connection_set_type_changed_cb(GetConnectionHandle(), OnNetworkChangedCb, nullptr);
        LOGD("Added callback for NETWORK");
        m_network_listener = callback;
    }
}

void SystemInfoListeners::UnregisterNetworkListener()
{
    if (nullptr != m_network_listener) {

        LOGD("Removed callback for NETWORK");
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
            LOGD("No need to register ip listener on platform, already registered");
        }
        LOGD("Added callback for WIFI_NETWORK");
        m_wifi_network_listener = callback;
    }
}

void SystemInfoListeners::UnregisterWifiNetworkListener()
{
    //unregister if is wifi callback, but no cellular callback
    if (nullptr != m_wifi_network_listener && nullptr == m_cellular_network_listener) {
        UnregisterIpChangeCallback();
        LOGD("Removed callback for WIFI_NETWORK");
    } else {
        LOGD("Removed callback for WIFI_NETWORK, but cellular listener still works");
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
            LOGD("No need to register ip listener on platform, already registered");
        }
        RegisterVconfCallback(VCONFKEY_TELEPHONY_FLIGHT_MODE,
                OnCellularNetworkValueChangedCb);
        RegisterVconfCallback(VCONFKEY_TELEPHONY_CELLID,
                OnCellularNetworkValueChangedCb);
        RegisterVconfCallback(VCONFKEY_TELEPHONY_LAC,
                OnCellularNetworkValueChangedCb);
        RegisterVconfCallback(VCONFKEY_TELEPHONY_SVC_ROAM,
                OnCellularNetworkValueChangedCb);
        LOGD("Added callback for CELLULAR_NETWORK");
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
            LOGD("Removed callback for CELLULAR_NETWORK");
        } else {
            LOGD("Removed callback for CELLULAR_NETWORK, but cellular listener still works");
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
        LOGD("Added callback for PERIPHERAL");
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
        LOGD("Removed callback for PERIPHERAL");
        m_peripheral_listener = nullptr;
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
    LOGD("");
    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdCpu);

    if (m_cpu_load == m_last_cpu_load) {
        return;
    }
    if (nullptr != m_cpu_listener) {
        m_last_cpu_load = m_cpu_load;
        m_cpu_listener();
    }
}

void SystemInfoListeners::OnStorageChangedCallback(void* /*event_ptr*/)
{
    LOGD("");
    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage);

    if (m_available_capacity_internal == m_last_available_capacity_internal) {
        return;
    }

    if (nullptr != m_storage_listener) {
        m_last_available_capacity_internal = m_available_capacity_internal;
        m_storage_listener();
    }
}

void SystemInfoListeners::OnMmcChangedCallback(keynode_t* /*node*/, void* /*event_ptr*/)
{
    LOGD("");
    picojson::value result = SysteminfoUtils::GetPropertyValue(kPropertyIdStorage);

    if (m_available_capacity_mmc == m_last_available_capacity_mmc) {
        return;
    }
    if (nullptr != m_storage_listener) {
        m_last_available_capacity_mmc = m_available_capacity_mmc;
        m_storage_listener();
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

TapiHandle* SystemInfoListeners::GetTapiHandle()
{
    if (nullptr == m_tapi_handle){
        m_tapi_handle = tel_init(0);
        if (nullptr == m_tapi_handle) {
            LOGE("Failed to connect with tapi, handle is null");
        }
    }
    return m_tapi_handle;
}


connection_h SystemInfoListeners::GetConnectionHandle()
{
    if (nullptr == m_connection_handle) {
        int error = connection_create(&m_connection_handle);
        if (CONNECTION_ERROR_NONE != error) {
            LOGE("Failed to create connection: %d", error);
            throw UnknownException("Cannot create connection");
        }
    }
    return m_connection_handle;
}

//////////////// Private ////////////////////

void SystemInfoListeners::RegisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
    if (0 != vconf_notify_key_changed(in_key, cb, nullptr)) {
        LOGE("Failed to register vconf callback: %s", in_key);
        throw UnknownException("Failed to register vconf callback");
    }
}

void SystemInfoListeners::UnregisterVconfCallback(const char *in_key, vconf_callback_fn cb)
{
    if (0 != vconf_ignore_key_changed(in_key, cb)) {
        LOGE("Failed to unregister vconf callback: %s", in_key);
        throw UnknownException("Failed to unregister vconf callback");
    }
}

void SystemInfoListeners::RegisterIpChangeCallback()
{
    LOGD("Registering connection callback");
    connection_h handle = GetConnectionHandle();
    int error = connection_set_ip_address_changed_cb(handle,
            OnNetworkValueChangedCb, nullptr);
    if (CONNECTION_ERROR_NONE != error) {
        LOGE("Failed to register ip change callback: %d", error);
        throw UnknownException("Cannot register ip change callback");
    }
}

void SystemInfoListeners::UnregisterIpChangeCallback()
{
    LOGD("Unregistering connection callback");
    connection_h handle = GetConnectionHandle();
    int error = connection_unset_ip_address_changed_cb(handle);
    if (CONNECTION_ERROR_NONE != error) {
        LOGE("Failed to unregister ip change callback: %d", error);
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
    LOGD("");
    system_info_listeners.OnBatteryChangedCallback(node, event_ptr);
}

gboolean OnCpuChangedCb(gpointer event_ptr)
{
    LOGD("");
    system_info_listeners.OnCpuChangedCallback(event_ptr);
    return G_SOURCE_CONTINUE;
}
gboolean OnStorageChangedCb(gpointer event_ptr)
{
    LOGD("");
    system_info_listeners.OnStorageChangedCallback(event_ptr);
    return G_SOURCE_CONTINUE;
}
void OnMmcChangedCb(keynode_t* node, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnMmcChangedCallback(node, event_ptr);
}

void OnDisplayChangedCb(keynode_t* node, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnDisplayChangedCallback(node, event_ptr);
}

void OnDeviceAutoRotationChangedCb(keynode_t* node, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnDeviceAutoRotationChangedCallback(node, event_ptr);
}

void OnDeviceOrientationChangedCb()
{
    LOGD("");
    system_info_listeners.OnDeviceOrientationChangedCallback();
}

void OnLocaleChangedCb(runtime_info_key_e key, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnLocaleChangedCallback(key, event_ptr);
}

void OnNetworkChangedCb(connection_type_e type, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnNetworkChangedCallback(type, event_ptr);
}

void OnNetworkValueChangedCb(const char* ipv4_address,
            const char* ipv6_address, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnNetworkValueCallback(ipv4_address, ipv6_address, event_ptr);
}

void OnCellularNetworkValueChangedCb(keynode_t *node, void *event_ptr)
{
    LOGD("");
    system_info_listeners.OnCellularNetworkValueCallback(node, event_ptr);
}

void OnPeripheralChangedCb(keynode_t* node, void* event_ptr)
{
    LOGD("");
    system_info_listeners.OnPeripheralChangedCallback(node, event_ptr);
}

/////////////////////////// SysteminfoUtils ////////////////////////////////

static bool GetValueBool(const char *key) {
    bool value = false;

    int ret = system_info_get_platform_bool(key, &value);
    if (SYSTEM_INFO_ERROR_NONE != ret) {
        std::string log_msg = "Platform error while getting bool value: ";
        log_msg += std::string(key) + " " + std::to_string(ret);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }

    LOGD("value[%s]: %s", key, value ? "true" : "false");
    return value;
}

static int GetValueInt(const char *key) {
    int value = 0;

    int ret = system_info_get_platform_int(key, &value);
    if (SYSTEM_INFO_ERROR_NONE != ret) {
        std::string log_msg = "Platform error while getting int value: ";
        log_msg += std::string(key) + " " + std::to_string(ret);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }

    LOGD("value[%s]: %d", key, value);
    return value;
}

static std::string GetValueString(const char *key) {
    char* value = nullptr;
    std::string str_value = "";

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
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }

    LOGD("value[%s]: %s", key, str_value.c_str());
    return str_value;
}

static std::string GetRuntimeInfoString(runtime_info_key_e key) {
    char* platform_c_string;
    int err = runtime_info_get_value_string(key, &platform_c_string);
    if (RUNTIME_INFO_ERROR_NONE == err) {
        if (nullptr != platform_c_string) {
            std::string platform_string = platform_c_string;
            free(platform_c_string);
            return platform_string;
        }
    }
    const char* error_msg = "Error when retrieving runtime information: " + err;
    LOGE("%s", error_msg);
    throw UnknownException(error_msg);
}

static std::string GetSystemValueString(system_info_key_e key) {
    char* platform_c_string;
    if (SYSTEM_INFO_ERROR_NONE
            == system_info_get_value_string(key, &platform_c_string)) {
        if (platform_c_string) {
            LOGD("Build platfrom string %s", platform_c_string);
            std::string platform_string = platform_c_string;
            free(platform_c_string);
            return platform_string;
        }
    }

    const char* error_msg = "Error when retrieving value from platform API";
    LOGE("%s", error_msg);
    throw UnknownException(error_msg);
}

int GetVconfInt(const char *key) {
    int value = 0;

    if (0 == vconf_get_int(key, &value)) {
            return value;
    } else {
        const std::string error_msg = "Could not get " + std::string(key);
        LOGD("%s",error_msg.c_str());
        throw UnknownException(error_msg);
    }

    LOGD("value[%s]: %d", key, value);
    return value;
}

picojson::value SysteminfoUtils::GetPropertyValue(const std::string& property)
{
    LOGD("Entered getPropertyValue");
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();
    if ("BATTERY" == property){
        ReportBattery(result_obj);
    } else if ("CPU" == property) {
        ReportCpu(result_obj);
    } else if ("STORAGE" == property) {
        ReportStorage(result_obj);
    } else if ("DISPLAY" == property) {
        ReportDisplay(result_obj);
    } else if ("DEVICE_ORIENTATION" == property) {
        ReportDeviceOrientation(result_obj);
    } else if ("BUILD" == property) {
        ReportBuild(result_obj);
    } else if ("LOCALE" == property) {
        ReportLocale(result_obj);
    } else if ("NETWORK" == property) {
        ReportNetwork(result_obj);
    } else if ("WIFI_NETWORK" == property) {
        ReportWifiNetwork(result_obj);
    } else if ("CELLULAR_NETWORK" == property) {
        ReportCellularNetwork(result_obj);
    } else if ("SIM" == property) {
        ReportSim(result_obj);
    } else if ("PERIPHERAL" == property) {
        ReportPeripheral(result_obj);
    } else {
        LOGD("Property with given id is not supported");
        throw NotSupportedException("Property with given id is not supported");
    }
    return result;
}

void SysteminfoUtils::ReportBattery(picojson::object& out) {
    int value = 0;
    int ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CAPACITY, &value);
    if (kVconfErrorNone != ret) {
        std::string log_msg = "Platform error while getting battery detail: ";
        LOGE("%s%d", log_msg.c_str(), ret);
        throw UnknownException((log_msg + std::to_string(ret)));
    }
    out.insert(std::make_pair("level", static_cast<double>(value)/kRemainingBatteryChargeMax));

    value = 0;
    ret = vconf_get_int(VCONFKEY_SYSMAN_BATTERY_CHARGE_NOW, &value);
    if (kVconfErrorNone != ret) {
        std::string log_msg =  "Platform error while getting battery charging: ";
        LOGE("%s%d",log_msg.c_str(), ret);
        throw UnknownException((log_msg + std::to_string(ret)));
    }
    out.insert(std::make_pair("isCharging", picojson::value(0 != value)));
}
//TODO maybe make two functions later onGSourceFunc
void SysteminfoUtils::ReportCpu(picojson::object& out) {
    LOGD("enter");
    static CpuInfo cpu_info;
    FILE *fp = nullptr;
    fp = fopen("/proc/stat", "r");
    if (nullptr == fp) {
        std::string error_msg("Can not open /proc/stat for reading");
        LOGE( "%s", error_msg.c_str() );
        throw UnknownException( error_msg );
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
            LOGW("Cannot calculate cpu load, previous value returned");
            load = cpu_info.load;
        }
    } else {
        std::string error_msg( "Could not read /proc/stat" );
        LOGE( "%s", error_msg.c_str() );
        throw UnknownException( error_msg );
    }

    system_info_listeners.SetCpuInfoLoad(cpu_info.load);

    load = 100 - load;
    LOGD("Cpu load : %f", load );
    out.insert(std::make_pair("load", load / 100.0));
}

void SysteminfoUtils::ReportDisplay(picojson::object& out) {
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
        LOGE("Cannot get value of screen width");
        throw UnknownException("Cannot get value of screen width");
    }
    if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
            SYSTEM_INFO_KEY_SCREEN_HEIGHT, &screenHeight)) {
        LOGE("Cannot get value of screen height");
        throw UnknownException("Cannot get value of screen height");
    }
    out.insert(std::make_pair("resolutionWidth", std::to_string(screenWidth)));
    out.insert(std::make_pair("resolutionHeight", std::to_string(screenHeight)));

    //FETCH DOTS PER INCH
    int dots_per_inch=0;
    if (SYSTEM_INFO_ERROR_NONE == system_info_get_platform_int(
            "tizen.org/feature/screen.dpi", &dots_per_inch)) {
        dotsPerInchWidth = dots_per_inch;
        dotsPerInchHeight = dots_per_inch;
    } else {
        LOGE("Cannot get 'tizen.org/feature/screen.dpi' value");
        throw UnknownException("Cannot get 'tizen.org/feature/screen.dpi' value");
    }
    out.insert(std::make_pair("dotsPerInchWidth", std::to_string(dotsPerInchWidth)));
    out.insert(std::make_pair("dotsPerInchHeight", std::to_string(dotsPerInchHeight)));

    //FETCH PHYSICAL WIDTH
    if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
            SYSTEM_INFO_KEY_PHYSICAL_SCREEN_WIDTH, &physicalWidth)) {
        LOGE("Cannot get value of phisical screen width");
        throw UnknownException("Cannot get value of phisical screen width");
    }
    out.insert(std::make_pair("physicalWidth", std::to_string(physicalWidth)));

    //FETCH PHYSICAL HEIGHT
    if (SYSTEM_INFO_ERROR_NONE != system_info_get_value_int(
            SYSTEM_INFO_KEY_PHYSICAL_SCREEN_HEIGHT, &physicalHeight)) {
        LOGE("Cannot get value of phisical screen height");
        throw UnknownException("Cannot get value of phisical screen height");
    }
    out.insert(std::make_pair("physicalHeight", std::to_string(physicalHeight)));

    //FETCH BRIGHTNESS
    int brightness;
    if (kVconfErrorNone == vconf_get_int(VCONFKEY_SETAPPL_LCD_BRIGHTNESS, &brightness)) {
        scaledBrightness = static_cast<double>(brightness)/kDisplayBrightnessDivideValue;
    } else {
        LOGE("Cannot get brightness value of display");
        throw UnknownException("Cannot get brightness value of display");
    }
    out.insert(std::make_pair("brightness", scaledBrightness));
}

void SysteminfoUtils::ReportDeviceOrientation(picojson::object& out) {
    SystemInfoDeviceOrientationPtr dev_orientation =
                SystemInfoDeviceOrientationPtr(new SystemInfoDeviceOrientation());
    std::string status = dev_orientation->status();
    bool auto_rotation_bool = dev_orientation->is_auto_rotation();
    out.insert(std::make_pair("isAutoRotation", auto_rotation_bool));
    out.insert(std::make_pair("status", status));
}

void SysteminfoUtils::ReportBuild(picojson::object& out) {
    std::string model = GetValueString("tizen.org/system/model_name");
    out.insert(std::make_pair("model", model));

    std::string manufacturer = GetSystemValueString(SYSTEM_INFO_KEY_MANUFACTURER);
    out.insert(std::make_pair("manufacturer", manufacturer));
    std::string buildVersion = GetSystemValueString(SYSTEM_INFO_KEY_BUILD_STRING);
    out.insert(std::make_pair("buildVersion", buildVersion));
}

void SysteminfoUtils::ReportLocale(picojson::object& out) {
    std::string str_language = GetRuntimeInfoString(RUNTIME_INFO_KEY_LANGUAGE);
    out.insert(std::make_pair("language", str_language));

    std::string str_country = GetRuntimeInfoString(RUNTIME_INFO_KEY_REGION);;
    out.insert(std::make_pair("country", str_country));
}

static std::string GetNetworkTypeString(NetworkType type)
{
    switch (type) {
        case kNone:
            return kNetworkTypeNone;
        case kType2G:
            return kNetworkType2G;
        case kType2_5G:
            return kNetworkType2_5G;
        case kType3G:
            return kNetworkType3G;
        case kType4G:
            return kNetworkType4G;
        case kWifi:
            return kNetworkTypeWifi;
        case kEthernet:
            return kNetworkTypeEthernet;
        case kUnknown:
            return kNetworkTypeUnknown;
        default:
            LOGE("Incorrect type: %d", type);
            throw TypeMismatchException("Incorrect type");
    }
}

void SysteminfoUtils::ReportNetwork(picojson::object& out) {
    connection_h connection_handle = nullptr;
    connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
    int networkType = 0;
    NetworkType type = kNone;

    //connection must be created in every call, in other case error occurs
    int error = connection_create(&connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
        std::string log_msg = "Cannot create connection: " + std::to_string(error);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }
    std::unique_ptr<std::remove_pointer<connection_h>::type, int(*)(connection_h)>
            connection_handle_ptr(connection_handle, &connection_destroy);
            // automatically release the memory

    error = connection_get_type(connection_handle, &connection_type);
    if (CONNECTION_ERROR_NONE != error) {
        std::string log_msg = "Cannot get connection type: " + std::to_string(error);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
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
        LOGE("Incorrect type: %d", connection_type);
        throw UnknownException("Incorrect type");
    }
    out.insert(std::make_pair("networkType", GetNetworkTypeString(type)));
}

static void GetIps(connection_profile_h profile_handle, std::string* ip_addr_str,
        std::string* ipv6_addr_str){
    //getting ipv4 address
    char* ip_addr = nullptr;
    int error = connection_profile_get_ip_address(profile_handle,
            CONNECTION_ADDRESS_FAMILY_IPV4,
            &ip_addr);
    if (CONNECTION_ERROR_NONE != error) {
        LOGE("Failed to get ip address: %d", error);
        throw UnknownException("Cannot get ip address");
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
        LOGE("Failed to get ipv6 address: %d", error);
        throw UnknownException("Cannot get ipv6 address");
    }
}

void SysteminfoUtils::ReportWifiNetwork(picojson::object& out) {
    bool result_status = false;
    std::string result_ssid;
    std::string result_ip_address;
    std::string result_ipv6_address;
    double result_signal_strength = 0;

    connection_h connection_handle = nullptr;
    connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
    connection_profile_h profile_handle = nullptr;

    //connection must be created in every call, in other case error occurs
    int error = connection_create(&connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
        std::string log_msg = "Cannot create connection: " + std::to_string(error);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }
    std::unique_ptr<std::remove_pointer<connection_h>::type, int(*)(connection_h)>
            connection_handle_ptr(connection_handle, &connection_destroy);
            // automatically release the memory

    error = connection_get_type(connection_handle, &connection_type);
    if (CONNECTION_ERROR_NONE != error) {
        std::string log_msg = "Cannot get connection type: " + std::to_string(error);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }
    if (CONNECTION_TYPE_WIFI == connection_type) {
        result_status = true;
        //gathering profile
        error = connection_get_current_profile(connection_handle, &profile_handle);
        if (CONNECTION_ERROR_NONE != error) {
            std::string log_msg = "Cannot get connection profile: " + std::to_string(error);
            LOGE("%s", log_msg.c_str());
            throw UnknownException(log_msg);
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
            LOGE("%s", log_msg.c_str());
            throw UnknownException(log_msg);
        }

        //gathering ips
        GetIps(profile_handle, &result_ip_address, &result_ipv6_address);

        //gathering strength
        int rssi = 0;
        error = connection_profile_get_wifi_rssi(profile_handle, &rssi);
        if (CONNECTION_ERROR_NONE == error) {
            result_signal_strength = (double) rssi/kWifiSignalStrengthDivideValue;
        }
        else {
            std::string log_msg = "Failed to get signal strength: " + std::to_string(error);
            LOGE("%s", log_msg.c_str());
            throw UnknownException(log_msg);
        }
    }
    else {
        LOGD("Connection type = %d. WIFI is disabled", connection_type);
    }

    out.insert(std::make_pair("status", result_status ? kWifiStatusOn : kWifiStatusOff));
    out.insert(std::make_pair("ssid", result_ssid));
    out.insert(std::make_pair("ipAddress", result_ip_address));
    out.insert(std::make_pair("ipv6Address", result_ipv6_address));
    out.insert(std::make_pair("signalStrength", std::to_string(result_signal_strength)));
}

static void FetchVconfSettings(
        unsigned short *result_mcc,
        unsigned short *result_mnc,
        unsigned short *result_cell_id,
        unsigned short *result_lac,
        bool *result_is_roaming,
        bool *result_is_flight_mode)
{
    LOGD("Entered");
    int result;
    if (0 != vconf_get_int(VCONFKEY_TELEPHONY_PLMN, &result)) {
        LOGE("Cannot get mcc value");
        throw UnknownException("Cannot get mcc value");
    }
    *result_mcc = static_cast<unsigned short>(result) / kMccDivider;
    *result_mnc = static_cast<unsigned short>(result) % kMccDivider;

    if (0 != vconf_get_int(VCONFKEY_TELEPHONY_CELLID, &result)) {
        LOGE("Cannot get cell_id value");
        throw UnknownException("Cannot get cell_id value");
    }
    *result_cell_id = static_cast<unsigned short>(result);

    if (0 != vconf_get_int(VCONFKEY_TELEPHONY_LAC, &result)) {
        LOGE("Cannot get lac value");
        throw UnknownException("Cannot get lac value");
    }
    *result_lac = static_cast<unsigned short>(result);

    if (0 != vconf_get_int(VCONFKEY_TELEPHONY_SVC_ROAM, &result)) {
        LOGE("Cannot get is_roaming value");
        throw UnknownException("Cannot get is_roaming value");
    }
    *result_is_roaming = (0 != result) ? true : false;

    if (0 != vconf_get_bool(VCONFKEY_TELEPHONY_FLIGHT_MODE, &result)) {
        LOGE("Cannot get is_flight_mode value");
        throw UnknownException("Cannot get is_flight_mode value");
    }
    *result_is_flight_mode = (0 != result) ? true : false;
}

static void FetchConnection(TapiHandle *tapi_handle, std::string* result_status,
        std::string* result_apn, std::string* result_ip_address,
        std::string* result_ipv6_address, std::string* result_imei)
{
    LOGD("Entered");
    connection_type_e connection_type = CONNECTION_TYPE_DISCONNECTED;
    connection_profile_h profile_handle = nullptr;
    connection_h connection_handle = nullptr;

    //connection must be created in every call, in other case error occurs
    int error = connection_create(&connection_handle);
    if (CONNECTION_ERROR_NONE != error) {
        std::string log_msg = "Cannot create connection: " + std::to_string(error);
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }
    std::unique_ptr<std::remove_pointer<connection_h>::type, int(*)(connection_h)>
            connection_handle_ptr(connection_handle, &connection_destroy);
            // automatically release the memory

    error = connection_get_type(connection_handle, &connection_type);
    if (CONNECTION_ERROR_NONE != error) {
        LOGE("Failed to get connection type: %d", error);
        throw UnknownException("Cannot get connection type");
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
            LOGE("Failed to get profile: %d", error);
            throw UnknownException("Cannot get profile");
        }

        error = connection_profile_get_cellular_apn(profile_handle, &apn);
        if (CONNECTION_ERROR_NONE != error) {
            LOGE("Failed to get apn name: %d", error);
            throw UnknownException("Cannot get apn name");
        }
        *result_apn = apn;
        free(apn);

        GetIps(profile_handle, result_ip_address, result_ipv6_address);
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
                LOGE("Failed to get default apn name: %d. Failing silently",
                        error);
            }
        } else {
            LOGE("Failed to get default profile: %d. Failing silently",
                    error);
        }
    }

    char* imei = nullptr;
    imei = tel_get_misc_me_imei_sync(tapi_handle);
    if (nullptr != imei) {
        *result_imei = imei;
        free(imei);
    } else {
        LOGE("Failed to get imei, nullptr pointer. Setting empty value.");
        *result_imei = "";
    }
}

void SysteminfoUtils::ReportCellularNetwork(picojson::object& out) {
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
    FetchVconfSettings(&result_mcc, &result_mnc, &result_cell_id, &result_lac,
            &result_is_roaming, &result_is_flight_mode);
    //gathering connection informations
    FetchConnection(system_info_listeners.GetTapiHandle(),
            &result_status, &result_apn, &result_ip_address, &result_ipv6_address, &result_imei);

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
}

void SimCphsValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
    LOGD("Entered");
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
        LOGW("Failed to retrieve cphs_info: %d", access_rt);
    }
    sim_mgr.set_operator_name(result_operator);
    sim_mgr.TryReturn();
}

void SimMsisdnValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
    LOGD("Entered");
    TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
    TelSimMsisdnList_t *msisdn_info = static_cast<TelSimMsisdnList_t*>(data);

    std::string result_msisdn;
    if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
        if (msisdn_info->count > 0) {
            if (nullptr != msisdn_info->list[0].num) {
                result_msisdn = msisdn_info->list[0].num;
            } else {
                LOGW("MSISDN number empty");
            }
        } else {
            LOGW("msisdn_info list empty");
        }
    } else {
        LOGW("Failed to retrieve msisdn_: %d", access_rt);
    }

    sim_mgr.set_msisdn(result_msisdn);
    sim_mgr.TryReturn();
}

void SimSpnValueCallback(TapiHandle */*handle*/, int result, void *data, void */*user_data*/)
{
    LOGD("Entered");
    TelSimAccessResult_t access_rt = static_cast<TelSimAccessResult_t>(result);
    TelSimSpn_t *spn_info = static_cast<TelSimSpn_t*>(data);

    std::string result_spn;
    if (TAPI_SIM_ACCESS_SUCCESS == access_rt) {
        result_spn = (char *)spn_info->spn;
    } else {
        LOGW("Failed to retrieve spn_: %d", access_rt);
    }

    sim_mgr.set_spn(result_spn);
    sim_mgr.TryReturn();
}

void SysteminfoUtils::ReportSim(picojson::object& out) {

    sim_mgr.GatherSimInformation(system_info_listeners.GetTapiHandle(), &out);
}

void SysteminfoUtils::ReportPeripheral(picojson::object& out) {
    try {
        int wireless_display_status = GetVconfInt(VCONFKEY_MIRACAST_WFD_SOURCE_STATUS);
        if (VCONFKEY_MIRACAST_WFD_SOURCE_ON == wireless_display_status) {
            out.insert(std::make_pair(kVideoOutputString, true));
            return;
        }
    } catch (const PlatformException& e) {
        // empty on purpose
    }

    try {
        int hdmi_status = GetVconfInt(VCONFKEY_SYSMAN_HDMI);
        if (VCONFKEY_SYSMAN_HDMI_CONNECTED == hdmi_status) {
            out.insert(std::make_pair(kVideoOutputString, true));
            return;
        }
    } catch (const PlatformException& e) {
        // empty on purpose
    }

    try {
        int popsync_status = GetVconfInt(VCONFKEY_POPSYNC_ACTIVATED_KEY);
        if (1 == popsync_status) {
            out.insert(std::make_pair(kVideoOutputString, true));
            return;
        }
    } catch (const PlatformException& e) {
        // empty on purpose
    }

    out.insert(std::make_pair(kVideoOutputString, false));
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

void SysteminfoUtils::ReportStorage(picojson::object& out) {
    int sdcardState = 0;
    struct statfs fs;

    picojson::value result = picojson::value(picojson::array());

    picojson::array& array = result.get<picojson::array>();
    array.push_back(picojson::value(picojson::object()));
    picojson::object& internal_obj = array.back().get<picojson::object>();

    if (statfs(kStorageInternalPath, &fs) < 0) {
        LOGE("There are no storage units detected");
        throw UnknownException("There are no storage units detected");
    }
    CreateStorageInfo(kTypeInternal, fs, &internal_obj);
    system_info_listeners.SetAvailableCapacityInternal(fs.f_bavail);

    if (0 == vconf_get_int(VCONFKEY_SYSMAN_MMC_STATUS, &sdcardState)) {
        if (VCONFKEY_SYSMAN_MMC_MOUNTED == sdcardState){
            if (statfs(kStorageSdcardPath, &fs) < 0) {
                LOGE("MMC mounted, but not accessible");
                throw UnknownException("MMC mounted, but not accessible");
            }
            array.push_back(picojson::value(picojson::object()));
            picojson::object& external_obj = array.back().get<picojson::object>();
            CreateStorageInfo(kTypeMmc, fs, &external_obj);
            system_info_listeners.SetAvailableCapacityMmc(fs.f_bavail);
        }
    }

    out.insert(std::make_pair("storages", result));
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

static bool CheckStringCapability(const std::string& key, std::string* value)
{
    LOGD("Entered CheckStringCapability");
    if (key == kTizenFeatureOpenglesTextureFormat) {
        *value = SystemInfoDeviceCapability::GetOpenglesTextureFormat();
    } else if (key == kTizenFeatureCoreApiVersion) {
        *value = "2.3";
    } else if (key == kTizenFeaturePlatfromCoreCpuArch) {
        *value = SystemInfoDeviceCapability::GetPlatfomCoreCpuArch();
    } else if (key == kTizenFeaturePlatfromCoreFpuArch) {
        *value = SystemInfoDeviceCapability::GetPlatfomCoreFpuArch();
    } else if (key == kTizenFeatureProfile) {
        *value = SystemInfoDeviceCapability::GetProfile();
    } else if (key == kTizenSystemDuid) {
        *value = SystemInfoDeviceCapability::GetDuid();
    } else if (key == kTizenFeatureInputKeyboardLayout ||
            key == kTizenFeatureNativeApiVersion ||
            key == kTizenSystemPlatformVersion ||
            key == kTizenSystemPlatformWebApiVersion ||
            key == kTizenSystemPlatformName) {
        try {
            *value = GetValueString(key.substr(strlen("http://")).c_str());
        } catch (...){
            return false;
        }
    } else {
        return false;
    }
    return true;
}

static bool CheckBoolCapability(const std::string& key, bool* bool_value)
{
    LOGD("Entered CheckBoolCapability");
    bool fetched = false;
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
        *bool_value = SystemInfoDeviceCapability::IsScreenSize320_320();
        fetched = true;
    } else {
        try {
            *bool_value = GetValueBool(key.substr(strlen("http://")).c_str());
            fetched = true;
        } catch (...){
            //empty for purpose - ignore that key was not found
        }
    }
    return fetched;
}

static bool CheckIntCapability(const std::string& key, std::string* value)
{
    LOGD("Entered CheckIntCapability");
    if (key == kTizenFeatureMultitouchCount ||
            key == kTizenFeatureScreenBpp ||
            key == kTizenFeatureScreenDpi ||
            key == kTizenFeatureScreenHeight ||
            key == kTizenFeatureScreenWidth) {
        try {
            *value = std::to_string(GetValueInt(key.substr(strlen("http://")).c_str()));
            return true;
        } catch (...) {
            //empty for purpose - ignore that key was not found
        }
    }
    return false;
}

///////////////////////   SystemInfoDeviceCapability   //////////////////////////////////////
picojson::value SystemInfoDeviceCapability::GetCapability(const std::string& key)
{
    picojson::value result = picojson::value(picojson::object());
    picojson::object& result_obj = result.get<picojson::object>();

    std::string value = "";
    std::string type = "";
    bool bool_value = false ;
    if (CheckStringCapability(key, &value)) {
        type = "string";
    } else if (CheckIntCapability(key, &value)) {
        type = "int";
    } else if(CheckBoolCapability(key, &bool_value)) {
        type = "bool";
    }

    if (type == "bool") {
        result_obj.insert(std::make_pair("value", bool_value));
    } else if (type == "string" || type == "int") {
        result_obj.insert(std::make_pair("value", value));
    } else {
        LOGD("Value for given key was not found");
        throw UnknownException("Value for given key was not found");
    }
    result_obj.insert(std::make_pair("type", type));

    return result;
}

bool SystemInfoDeviceCapability::IsBluetooth() {
    return GetValueBool("tizen.org/feature/network.bluetooth");
}

bool SystemInfoDeviceCapability::IsNfc() {
    return GetValueBool("tizen.org/feature/network.nfc");
}

bool SystemInfoDeviceCapability::IsNfcReservedPush() {
    return GetValueBool("tizen.org/feature/network.nfc.reserved_push");
}

unsigned short SystemInfoDeviceCapability::GetMultiTouchCount() {
    return GetValueInt("tizen.org/feature/multi_point_touch.point_count");
}

bool SystemInfoDeviceCapability::IsInputKeyboard() {
    return GetValueBool("tizen.org/feature/input.keyboard");
}

bool SystemInfoDeviceCapability::IsInputKeyboardLayout() {
    std::string input_keyboard_layout =
            GetValueString("tizen.org/feature/input.keyboard.layout");

    bool input_keyboard = GetValueBool("tizen.org/feature/input.keyboard");

    // according to SystemInfo-DeviceCapabilities-dependency-table
    // inputKeyboard   inputKeyboardLayout
    //  O               O                   Possible
    //  O               X                   Possible
    //  X               X                   Possible
    //  X               O                   Impossible

    return input_keyboard ? !(input_keyboard_layout.empty()) : false;
}

bool SystemInfoDeviceCapability::IsWifi() {
    return GetValueBool("tizen.org/feature/network.wifi");
}

bool SystemInfoDeviceCapability::IsWifiDirect() {
    return GetValueBool("tizen.org/feature/network.wifi.direct");
}

bool SystemInfoDeviceCapability::IsFmRadio() {
    return GetValueBool("tizen.org/feature/fmradio");
}

bool SystemInfoDeviceCapability::IsOpengles() {
    return GetValueBool("tizen.org/feature/opengles");
}

bool SystemInfoDeviceCapability::IsOpenglesVersion11() {
    return GetValueBool("tizen.org/feature/opengles.version.1_1");
}

bool SystemInfoDeviceCapability::IsOpenglesVersion20() {
    return GetValueBool("tizen.org/feature/opengles.version.2_0");
}

std::string SystemInfoDeviceCapability::GetOpenglesTextureFormat() {
    if (!GetValueBool("tizen.org/feature/opengles")) {
        // this exception is converted to "Undefined" value in JS layer
        std::string log_msg = "OpenGL-ES is not supported";
        LOGE("%s", log_msg.c_str());
        throw NotSupportedException(log_msg);
    }
    std::string texture_format = "";
    if (GetValueBool("tizen.org/feature/opengles.texture_format.utc")) {
        texture_format += kOpenglesTextureUtc;
    }
    if (GetValueBool("tizen.org/feature/opengles.texture_format.ptc")) {
        if (!texture_format.empty()) {
            texture_format += kOpenglesTextureDelimiter;
        }
        texture_format += kOpenglesTexturePtc;
    }
    if (GetValueBool("tizen.org/feature/opengles.texture_format.etc")) {
        if (!texture_format.empty()) {
            texture_format += kOpenglesTextureDelimiter;
        }
        texture_format += kOpenglesTextureEtc;
    }
    if (GetValueBool("tizen.org/feature/opengles.texture_format.3dc")) {
        if (!texture_format.empty()) {
            texture_format += kOpenglesTextureDelimiter;
        }
        texture_format += kOpenglesTexture3dc;
    }
    if (GetValueBool("tizen.org/feature/opengles.texture_format.atc")) {
        if (!texture_format.empty()) {
            texture_format += kOpenglesTextureDelimiter;
        }
        texture_format += kOpenglesTextureAtc;
    }
    if (GetValueBool("tizen.org/feature/opengles.texture_format.pvrtc")) {
        if (!texture_format.empty()) {
            texture_format += kOpenglesTextureDelimiter;
        }
        texture_format += kOpenglesTexturePvrtc;
    }
    if (texture_format.empty()) {
        // this exception is converted to "Undefined" value in JS layer
        std::string log_msg = "Platform error while getting OpenGL-ES texture format";
        LOGE("%s", log_msg.c_str());
        throw UnknownException(log_msg);
    }
    return texture_format;
}

bool SystemInfoDeviceCapability::IsSpeechRecognition() {
    return GetValueBool("tizen.org/feature/speech.recognition");
}

bool SystemInfoDeviceCapability::IsSpeechSynthesis() {
    return GetValueBool("tizen.org/feature/speech.synthesis");
}

bool SystemInfoDeviceCapability::IsAccelerometer() {
    return GetValueBool("tizen.org/feature/sensor.accelerometer");
}

bool SystemInfoDeviceCapability::IsAccelerometerWakeup() {
    return GetValueBool("tizen.org/feature/sensor.accelerometer.wakeup");
}

bool SystemInfoDeviceCapability::IsBarometer() {
    return GetValueBool("tizen.org/feature/sensor.barometer");
}

bool SystemInfoDeviceCapability::IsBarometerWakeup() {
    return GetValueBool("tizen.org/feature/sensor.barometer.wakeup");
}

bool SystemInfoDeviceCapability::IsGyroscope() {
    return GetValueBool("tizen.org/feature/sensor.gyroscope");
}

bool SystemInfoDeviceCapability::IsGyroscopeWakeup() {
    return GetValueBool("tizen.org/feature/sensor.gyroscope.wakeup");
}

bool SystemInfoDeviceCapability::IsGraphicsAcceleration() {
    return GetValueBool("tizen.org/feature/graphics.acceleration");
}

bool SystemInfoDeviceCapability::IsPush() {
    return GetValueBool("tizen.org/feature/network.push");
}

bool SystemInfoDeviceCapability::IsTelephony() {
    return GetValueBool("tizen.org/feature/network.telephony");
}

bool SystemInfoDeviceCapability::IsTelephonyMMS() {
    return GetValueBool("tizen.org/feature/network.telephony.mms");
}

bool SystemInfoDeviceCapability::IsTelephonySMS() {
    return GetValueBool("tizen.org/feature/network.telephony.sms");
}

bool SystemInfoDeviceCapability::IsCamera() {
    return GetValueBool("tizen.org/feature/camera");
}

bool SystemInfoDeviceCapability::IsCameraFront() {
    return GetValueBool("tizen.org/feature/camera.front");
}

bool SystemInfoDeviceCapability::IsCameraFrontFlash() {
    return GetValueBool("tizen.org/feature/camera.front.flash");
}

bool SystemInfoDeviceCapability::IsCameraBack() {
    return GetValueBool("tizen.org/feature/camera.back");
}

bool SystemInfoDeviceCapability::IsCameraBackFlash() {
    return GetValueBool("tizen.org/feature/camera.back.flash");
}

bool SystemInfoDeviceCapability::IsLocation() {
    return GetValueBool("tizen.org/feature/location");
}

bool SystemInfoDeviceCapability::IsLocationGps() {
    return GetValueBool("tizen.org/feature/location.gps");
}

bool SystemInfoDeviceCapability::IsLocationWps() {
    return GetValueBool("tizen.org/feature/location.wps");
}

bool SystemInfoDeviceCapability::IsMicrophone() {
    return GetValueBool("tizen.org/feature/microphone");
}

bool SystemInfoDeviceCapability::IsUsbHost() {
    return GetValueBool("tizen.org/feature/usb.host");
}

bool SystemInfoDeviceCapability::IsUsbAccessory() {
    return GetValueBool("tizen.org/feature/usb.accessory");
}

bool SystemInfoDeviceCapability::IsScreenOutputRca() {
    return GetValueBool("tizen.org/feature/screen.output.rca");
}

bool SystemInfoDeviceCapability::IsScreenOutputHdmi() {
    return GetValueBool("tizen.org/feature/screen.output.hdmi");
}

std::string SystemInfoDeviceCapability::GetPlatfomCoreCpuArch() {
    std::string result;
    if (GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv6")) {
        result = kPlatformCoreArmv6;
    }
    if (GetValueBool("tizen.org/feature/platform.core.cpu.arch.armv7")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreArmv7;
    }
    if (GetValueBool("tizen.org/feature/platform.core.cpu.arch.x86")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreX86;
    }
    if (result.empty()) {
        LOGE("Platform error while retrieving platformCoreCpuArch: result is empty");
        throw UnknownException("platformCoreCpuArch result is empty");
    }
    return result;
}

std::string SystemInfoDeviceCapability::GetPlatfomCoreFpuArch() {
    std::string result;
    if (GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse2")) {
        result = kPlatformCoreSse2;
    }
    if (GetValueBool("tizen.org/feature/platform.core.fpu.arch.sse3")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreSse3;
    }
    if (GetValueBool("tizen.org/feature/platform.core.fpu.arch.ssse3")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreSsse3;
    }
    if (GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv2")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreVfpv2;
    }
    if (GetValueBool("tizen.org/feature/platform.core.fpu.arch.vfpv3")) {
        if (!result.empty()) {
            result += kPlatformCoreDelimiter;
        }
        result += kPlatformCoreVfpv3;
    }
    if (result.empty()) {
        LOGE("Platform error while retrieving platformCoreFpuArch: result is empty");
        throw UnknownException("platformCoreFpuArch result is empty");
    }
    return result;
}

bool SystemInfoDeviceCapability::IsSipVoip() {
    return GetValueBool("tizen.org/feature/sip.voip");
}

std::string SystemInfoDeviceCapability::GetPlatformName() {
    return GetValueString("tizen.org/system/platform.name");
}

std::string SystemInfoDeviceCapability::GetPlatformVersion() {
    return GetValueString("tizen.org/feature/platform.version");
}

std::string SystemInfoDeviceCapability::GetWebApiVersion() {
    return GetValueString("tizen.org/feature/platform.web.api.version");
}

bool SystemInfoDeviceCapability::IsMagnetometer() {
    return GetValueBool("tizen.org/feature/sensor.magnetometer");
}

bool SystemInfoDeviceCapability::IsMagnetometerWakeup() {
    return GetValueBool("tizen.org/feature/sensor.magnetometer.wakeup");
}

bool SystemInfoDeviceCapability::IsPhotometer() {
    return GetValueBool("tizen.org/feature/sensor.photometer");
}

bool SystemInfoDeviceCapability::IsPhotometerWakeup() {
    return GetValueBool("tizen.org/feature/sensor.photometer.wakeup");
}

bool SystemInfoDeviceCapability::IsProximity() {
    return GetValueBool("tizen.org/feature/sensor.proximity");
}

bool SystemInfoDeviceCapability::IsProximityWakeup() {
    return GetValueBool("tizen.org/feature/sensor.proximity.wakeup");
}

bool SystemInfoDeviceCapability::IsTiltmeter() {
    return GetValueBool("tizen.org/feature/sensor.tiltmeter");
}

bool SystemInfoDeviceCapability::IsTiltmeterWakeup() {
    return GetValueBool("tizen.org/feature/sensor.tiltmeter.wakeup");
}

bool SystemInfoDeviceCapability::IsDataEncryption() {
    return GetValueBool("tizen.org/feature/database.encryption");
}

bool SystemInfoDeviceCapability::IsAutoRotation() {
    return GetValueBool("tizen.org/feature/screen.auto_rotation");
}

bool SystemInfoDeviceCapability::IsVisionImageRecognition() {
    return GetValueBool("tizen.org/feature/vision.image_recognition");
}

bool SystemInfoDeviceCapability::IsVisionQrcodeGeneration() {
    return GetValueBool("tizen.org/feature/vision.qrcode_generation");
}

bool SystemInfoDeviceCapability::IsVisionQrcodeRecognition() {
    return GetValueBool("tizen.org/feature/vision.qrcode_recognition");
}

bool SystemInfoDeviceCapability::IsVisionFaceRecognition() {
    return GetValueBool("tizen.org/feature/vision.face_recognition");
}

bool SystemInfoDeviceCapability::IsSecureElement() {
    return GetValueBool("tizen.org/feature/network.secure_element");
}

std::string SystemInfoDeviceCapability::GetProfile() {
    std::string profile = GetValueString("tizen.org/feature/profile");
    if ( kPlatformFull == profile ) {
        return kProfileFull;
    } else if ( kPlatformMobile == profile ) {
        return kProfileMobile;
    } else if ( kPlatformWearable == profile ) {
        return kProfileWearable;
    }
    return kProfileFull;
}

std::string SystemInfoDeviceCapability::GetNativeAPIVersion()
{
    return GetValueString("tizen.org/feature/platform.native.api.version");
}

//Implementation ported from devel/webapi/refactoring branch,
//all flags are hardcoded for Kiran on top of this file
std::string SystemInfoDeviceCapability::GenerateDuid()
{
    LOGD("Enter");

    bool supported = false;
    std::string duid = "";
    char* device_string = nullptr;
    int ret = 0;

#ifdef FEATURE_OPTIONAL_TELEPHONY
    ret = system_info_get_platform_bool("tizen.org/feature/network.telephony", &supported);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
        LOGE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.telephony).");
        return duid;
    }
    LOGD("telephony is %s", supported ? "supported." : "not supported.");

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
                    LOGD("It is failed to get IMEI.");
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
                LOGD("Modem is not ready to get IMEI.");
                return duid;
            }

            LOGD("telephony.- device_string : %s", device_string);
        } else {
            LOGD("Modem handle is not ready.");
            return duid;
        }
    } else {
        LOGD("It is failed to generate DUID from telephony information");
    }

#elif FEATURE_OPTIONAL_BT
    ret = system_info_get_platform_bool("tizen.org/feature/network.bluetooth", &supported);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
        LOGE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.bluetooth).");
        return duid;
    }
    LOGD("bluetooth is %s", supported ? "supported." : "not supported.");

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

            LOGD("BT - device_string : %s", device_string);
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

            LOGD("BT - device_string : %s", device_string);
        } else {
            LOGD("fail file open.");
        }
#endif
    } else {
        LOGD("It is failed to generate DUID from bluetooth information");
    }

#elif FEATURE_OPTIONAL_WI_FI
    ret = system_info_get_platform_bool("tizen.org/feature/network.wifi", &supported);
    if (ret != SYSTEM_INFO_ERROR_NONE) {
        LOGE("It is failed to call system_info_get_platform_bool(tizen.org/feature/network.wifi).");
        return duid;
    }
    LOGD("Wi-Fi is %s", supported ? "supported." : "not supported.");

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

            LOGD("wifi - device_string : %s", pDeviceString);
        } else {
            LOGD("It is failed to get mac address");
        }
    } else {
        LOGD("It is failed to generate DUID from wi-fi information");
    }
#else
    LOGD("It is failed to generate DUID");
#endif

    if (device_string != nullptr) {
        duid = GenerateId(device_string);
        free(device_string);
    }
    device_string = nullptr;

    LOGD("duid : %s", duid.c_str());
    return duid;
}

std::string SystemInfoDeviceCapability::GenerateId(char* pDeviceString)
{
    LOGD("Enter");
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
    LOGD("Enter");

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

    LOGD("value : %llu", value);
}

std::string SystemInfoDeviceCapability::Base32Encode(byte* value)
{
    LOGD("Enter");

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

bool SystemInfoDeviceCapability::IsScreenSizeNormal()
{
    return GetValueBool("tizen.org/feature/screen.size.normal");
}

bool SystemInfoDeviceCapability::IsScreenSize480_800()
{
    return GetValueBool("tizen.org/feature/screen.size.normal.480.800");
}

bool SystemInfoDeviceCapability::IsScreenSize720_1280()
{
    return GetValueBool("tizen.org/feature/screen.size.normal.720.1280");
}

bool SystemInfoDeviceCapability::IsShellAppWidget()
{
    return GetValueBool("tizen.org/feature/shell.appwidget");
}

bool SystemInfoDeviceCapability::IsNativeOspCompatible()
{
    return GetValueBool("tizen.org/feature/platform.native.osp_compatible");
}

////additional capabilities
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

bool SystemInfoDeviceCapability::IsScreenSize320_320()
{
    int height = GetValueInt("tizen.org/feature/screen.height");
    int width = GetValueInt("tizen.org/feature/screen.width");
    if (height != 320 || width != 320) {
        return false;
    } else {
        return true;
    }
}

} // namespace systeminfo
} // namespace webapi
