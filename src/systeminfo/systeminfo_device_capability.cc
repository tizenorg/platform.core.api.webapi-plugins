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

#include "systeminfo_device_capability.h"

#include <fstream>
#include <system_info.h>

#include "common/logger.h"

// TODO:: hardcoded value, only for IsBluetoothAlwaysOn
#define PROFILE_MOBILE 1

namespace extension {
namespace systeminfo {

using common::PlatformResult;
using common::ErrorCode;

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
} //namespace

PlatformResult SystemInfoDeviceCapability::GetValueBool(const char *key, bool* value) {
  bool platform_result = false;
  int ret = system_info_get_platform_bool(key, &platform_result);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    ret = system_info_get_custom_bool(key, &platform_result);
    if (SYSTEM_INFO_ERROR_NONE != ret) {
      std::string log_msg = "Platform error while getting bool value: ";
      log_msg += std::string(key) + " " + std::to_string(ret);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
  }

  *value = platform_result;
  LoggerD("value[%s]: %s", key, *value ? "true" : "false");
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueInt(const char *key, int* value) {
  int platform_result = 0;
  int ret = system_info_get_platform_int(key, &platform_result);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    ret = system_info_get_custom_int(key, &platform_result);
    if (SYSTEM_INFO_ERROR_NONE != ret) {
      std::string log_msg = "Platform error while getting int value: ";
      log_msg += std::string(key) + " " + std::to_string(ret);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
  }

  *value = platform_result;
  LoggerD("value[%s]: %d", key, *value);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetValueString(const char *key, std::string* str_value) {
  char* value = nullptr;

  int ret = system_info_get_platform_string(key, &value);
  if (SYSTEM_INFO_ERROR_NONE != ret) {
    ret = system_info_get_custom_string(key, &value);
    if (SYSTEM_INFO_ERROR_NONE != ret) {
      std::string log_msg = "Platform error while getting string value: ";
      log_msg += std::string(key) + " " + std::to_string(ret);
      LoggerE("%s", log_msg.c_str());
      return PlatformResult(ErrorCode::UNKNOWN_ERR, log_msg);
    }
  }

  if (value != nullptr) {
    *str_value = value;
    free(value);
    value = nullptr;
  }

  LoggerD("value[%s]: %s", key, str_value->c_str());
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckStringCapability(const std::string& key, std::string* value, bool* fetched) {
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
    size_t prefix_len = strlen("http://");
    if (key.length() <= prefix_len) {
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Value for given key was not found");
    }
    PlatformResult ret = SystemInfoDeviceCapability::GetValueString(key.substr(prefix_len).c_str(), value);
    if (ret.IsError()){
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  *fetched = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckBoolCapability(const std::string& key, bool* bool_value, bool* fetched) {
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
    size_t prefix_len = strlen("http://");
    if (key.length() <= prefix_len) {
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Value for given key was not found");
    }
    PlatformResult ret = SystemInfoDeviceCapability::GetValueBool(
        key.substr(prefix_len).c_str(), bool_value);
    if (ret.IsSuccess()) {
      *fetched = true;
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult CheckIntCapability(const std::string& key, std::string* value,
                                         bool* fetched) {
  LoggerD("Entered CheckIntCapability");
  int result = 0;
  if (key == kTizenFeatureCpuFrequency) {
    PlatformResult ret = SystemInfoDeviceCapability::GetPlatformCoreCpuFrequency(&result);
    if (ret.IsError()) {
      *fetched = false;
      return ret;
    }
  } else {
    size_t prefix_len = strlen("http://");
    if (key.length() <= prefix_len) {
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, "Value for given key was not found");
    }
    PlatformResult ret = SystemInfoDeviceCapability::GetValueInt(
        key.substr(prefix_len).c_str(), &result);
    if (ret.IsError()) {
      *fetched = false;
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }
  *value = std::to_string(result);
  *fetched = true;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::GetCapability(const std::string& key,
                                                          picojson::value* result) {
  LoggerD("Entered");
  picojson::object& result_obj = result->get<picojson::object>();

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

bool SystemInfoDeviceCapability::IsScreen() {
  return true;
}

PlatformResult SystemInfoDeviceCapability::GetPlatformCoreCpuFrequency(int* return_value) {
  LoggerD("Entered");

  std::string freq;
  std::string file_name;

#ifdef TIZEN_IS_EMULATOR
  file_name = "/proc/cpuinfo";
#else
  file_name = "/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq";
#endif

  std::ifstream cpuinfo_freq(file_name);
  if (!cpuinfo_freq.is_open()) {
    LoggerE("Failed to get cpu frequency");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to open file");
  }

#ifdef TIZEN_IS_EMULATOR
  //get frequency value from cpuinfo file
  //example entry for frequency looks like below
  //cpu MHz   : 3392.046
  std::size_t found;
  do {
    getline(cpuinfo_freq, freq);
    found = freq.find("cpu MHz");
  } while (std::string::npos == found && !cpuinfo_freq.eof());

  found = freq.find(":");
  if (std::string::npos != found) {
    *return_value = std::stoi(freq.substr(found + 2));
  }
#else
  getline(cpuinfo_freq, freq);
  *return_value = std::stoi(freq) / 1000; // unit: MHz
#endif

  cpuinfo_freq.close();
  LoggerD("cpu frequency : %d", *return_value);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SystemInfoDeviceCapability::IsNativeOspCompatible(bool* result) {
  LoggerD("Enter");
#ifdef PROFILE_WEARABLE
  *result = false;
  return PlatformResult(ErrorCode::NO_ERROR);
#else
  return GetValueBool(kTizenFeaturePlatformNativeOspCompatible, result);
#endif
}

PlatformResult SystemInfoDeviceCapability::GetNativeAPIVersion(std::string* return_value) {
  LoggerD("Enter");
#ifdef PROFILE_WEARABLE
  *return_value = "";
  return PlatformResult(ErrorCode::NO_ERROR);
#else
  return GetValueString(kTizenFeaturePlatformNativeApiVersion, return_value);
#endif
}

PlatformResult SystemInfoDeviceCapability::GetPlatformVersionName(std::string* result) {
  LoggerD("Enter");

  //Because of lack of 'http://tizen.org/feature/platform.version.name'
  //key on platform we use 'http://tizen.org/system/platform.name'.
  return GetValueString("tizen.org/system/platform.name", result);
}

} // namespace systeminfo
} // namespace webapi
