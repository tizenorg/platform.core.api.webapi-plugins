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

#include "sound/sound_manager.h"

#include <tizen/tizen.h>
#include <vconf.h>
#include <vconf-keys.h>

#include "common/task-queue.h"

//This constant was originally defined in vconf.h. However, in tizen 3, it
//appears, it is removed (or defined only in vconf-internals.h)
//It is not clear, if it is final solution, or not.
#ifndef VCONF_OK
#define VCONF_OK 0
#endif

#include "sound/sound_instance.h"
#include "common/logger.h"
#include "common/converter.h"

namespace extension {
namespace sound {

using namespace common;
using namespace common::tools;

const std::map<std::string, sound_type_e> SoundManager::platform_enum_map_ = {
    {"SYSTEM", SOUND_TYPE_SYSTEM},
    {"NOTIFICATION", SOUND_TYPE_NOTIFICATION},
    {"ALARM", SOUND_TYPE_ALARM},
    {"MEDIA", SOUND_TYPE_MEDIA},
    {"VOICE", SOUND_TYPE_VOICE},
    {"RINGTONE", SOUND_TYPE_RINGTONE}};

PlatformResult SoundManager::StrToPlatformEnum(const std::string& key,
                                               sound_type_e* sound_type) {
  LoggerD("Enter");
  if (platform_enum_map_.find(key) == platform_enum_map_.end()) {
    std::string message = "Platform enum value not found for key " + key;
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
  }

  *sound_type = platform_enum_map_.at(key);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::PlatformEnumToStr(const sound_type_e value,
                                               std::string* sound_type) {
  LoggerD("Enter");
  for (auto& item : platform_enum_map_) {
    if (item.second == value) {
      *sound_type = item.first;

      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  std::string message =
      "Platform enum value " + std::to_string(value) + " not found";

  return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
}

std::string SoundManager::SoundDeviceTypeToString(sound_device_type_e type) {
  LoggerD("Enter");
  switch (type) {
    case SOUND_DEVICE_BUILTIN_SPEAKER:
      return "SPEAKER";
    case SOUND_DEVICE_BUILTIN_RECEIVER:
      return "RECEIVER";
    case SOUND_DEVICE_BUILTIN_MIC:
      return "MIC";
    case SOUND_DEVICE_AUDIO_JACK:
      return "AUDIO_JACK";
    case SOUND_DEVICE_BLUETOOTH:
      return "BLUETOOTH";
    case SOUND_DEVICE_HDMI:
      return "HDMI";
    case SOUND_DEVICE_MIRRORING:
      return "MIRRORING";
    case SOUND_DEVICE_USB_AUDIO:
      return "USB_AUDIO";
    default:
      LoggerE("Invalid sound_device_type_e: %d", type);
      return "";
  }
}

std::string SoundManager::SoundIOTypeToString(sound_device_io_direction_e type) {
  LoggerD("Enter");
  switch (type) {
    case SOUND_DEVICE_IO_DIRECTION_IN:
      return "IN";
    case SOUND_DEVICE_IO_DIRECTION_OUT:
      return "OUT";
    case SOUND_DEVICE_IO_DIRECTION_BOTH:
      return "BOTH";
    default:
      LoggerE("Invalid sound_device_io_direction_e: %d", type);
      return "";
  }
}

SoundManager::SoundManager(SoundInstance& instance)
    : soundModeChangeListening(false),
      sound_device_change_listener_(false),
      instance_(instance),
      soundModeListener(nullptr) {
  FillMaxVolumeMap();
}

SoundManager::~SoundManager() {
  LoggerD("Enter");
  if (soundModeChangeListening) {
    int status = vconf_ignore_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, SoundManager::soundModeChangedCb);
    if (VCONF_OK != status) {
      LoggerE("Cannot disable listener!");
    }
  }

  if (sound_device_change_listener_) {
    if (SOUND_MANAGER_ERROR_NONE != sound_manager_unset_device_connected_cb()) {
      LoggerE("Cannot unregister connection listener!");
    }

    if (SOUND_MANAGER_ERROR_NONE != sound_manager_unset_device_information_changed_cb()) {
      LoggerE("Cannot unregister information listener!");
    }
  }

  if (is_volume_change_listener_) {
    if (SOUND_MANAGER_ERROR_NONE != sound_manager_unset_volume_changed_cb()) {
      LoggerE("Cannot unregister volume listener!");
    }
  }
}

void SoundManager::FillMaxVolumeMap() {
  LoggerD("Enter");
  int max = 100;
  int ret;

  for (auto& item : platform_enum_map_) {
    max = 100;

    ret = sound_manager_get_max_volume(item.second, &max);
    if (ret != SOUND_MANAGER_ERROR_NONE) {
      LoggerE("SoundManagerGetMaxVolumeFailed : %d", ret);
    }

    LoggerD("maxVolume: %d - %d", item.second, max);

    max_volume_map_[item.second] = max;
  }
}

PlatformResult SoundManager::GetMaxVolume(sound_type_e type, int* max_volume) {
  LoggerD("Enter");
  auto it = max_volume_map_.find(type);
  if (it == max_volume_map_.end()) {
    std::string sound_type;
    PlatformResult status = PlatformEnumToStr(type, &sound_type);
    if (status.IsError()) return status;
    LoggerE("Failed to find maxVolume of type: %s", sound_type.c_str());

    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to find maxVolume");
  }

  *max_volume = it->second;

  return PlatformResult(ErrorCode::NO_ERROR);
}

double SoundManager::ConvertToSystemVolume(int max_volume, int volume) {
  LoggerD("Enter");
  return round(static_cast<double>(volume) * 10 / max_volume) / 10;
}

void SoundManager::VolumeChangeCallback(sound_type_e type, unsigned int value) {
  LoggerD("VolumeChangeCallback: type: %d, value: %d", type, value);

  // Prepare response
  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();

  response_obj.insert(
      std::make_pair("listenerId", picojson::value("VolumeChangeListener")));

  std::string sound_type;
  PlatformResult status = PlatformEnumToStr(type, &sound_type);
  if (status.IsError())
      return;

  response_obj.insert(
      std::make_pair("type", picojson::value(sound_type)));

  int max_volume;
  status = GetMaxVolume(type, &max_volume);
  if (status.IsError())
      return;

  response_obj.insert(std::make_pair(
      "volume",
      picojson::value(ConvertToSystemVolume(max_volume, value))));

  Instance::PostMessage(&instance_, response.serialize().c_str());
}

PlatformResult SoundManager::GetSoundMode(std::string* sound_mode_type) {
  LoggerD("Enter");
  int isEnableSound = 0;
  int isEnableVibrate = 0;

  *sound_mode_type = "MUTE";

  int ret = vconf_get_bool(VCONFKEY_SETAPPL_SOUND_STATUS_BOOL, &isEnableSound);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown error: " + std::to_string(ret));
  }

  ret =
      vconf_get_bool(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL, &isEnableVibrate);
  if (VCONF_OK != ret) {
    LoggerE("Unknown error : %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Unknown error: " + std::to_string(ret));
  }

  if (isEnableSound && isEnableVibrate) {
    LoggerE("Wrong state (sound && vibration)");
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Platform has wrong state.");
  }

  if (isEnableSound) {
    *sound_mode_type = "SOUND";
  } else if (isEnableVibrate) {
    *sound_mode_type = "VIBRATE";
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::SetVolume(const picojson::object& args) {
  LoggerD("Enter");
  const std::string& type = FromJson<std::string>(args, "type");
  double volume = FromJson<double>(args, "volume");

  LoggerD("SoundType: %s", type.c_str());
  LoggerD("volume: %f", volume);

  if (volume > 1.0 || volume < 0.0) {
    LoggerE("Volume should be the value between 0 and 1.");
    return PlatformResult(ErrorCode::INVALID_VALUES_ERR,
                          "Volume should be the value between 0 and 1.");
  }

  sound_type_e sound_type;
  PlatformResult status = SoundManager::StrToPlatformEnum(type, &sound_type);
  if (status.IsError()) return status;

  auto it = max_volume_map_.find(sound_type);
  if (it == max_volume_map_.end()) {
    LoggerE("Failed to find maxVolume of type: %d", type.c_str());
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to find maxVolume");
  }

  int max_volume = it->second;
  int value = round(volume * max_volume);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  int ret = sound_manager_set_volume(sound_type, value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to set volume: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to set volume");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::GetVolume(const picojson::object& args,
                                       double* volume) {
  LoggerD("Enter");
  const std::string& type = FromJson<std::string>(args, "type");
  int value = 0;

  sound_type_e type_enum;
  PlatformResult status = SoundManager::StrToPlatformEnum(type, &type_enum);
  if (status.IsError()) return status;

  int max_volume;
  status = GetMaxVolume(type_enum, &max_volume);
  if (status.IsError()) return status;

  int ret = sound_manager_get_volume(type_enum, &value);
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to get volume: %d", ret);
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get volume");
  }

  *volume = ConvertToSystemVolume(max_volume, value);
  LoggerD("volume: %lf, maxVolume: %d, value: %d", volume, max_volume, value);

  return PlatformResult(ErrorCode::NO_ERROR);
}

void SoundManager::soundModeChangedCb(keynode_t*, void* user_data)
{
  LoggerD("Enter");
  if (user_data == nullptr) {
    LoggerE("Invalid callback data!");
    return;
  }
  SoundManager* self = static_cast<SoundManager*>(user_data);

  std::string soundModeType;
  PlatformResult status = self->GetSoundMode(&soundModeType);

  if (status.IsSuccess() && self->soundModeListener) {
    self->soundModeListener->OnSoundModeChange(soundModeType);
  } else {
    LoggerE("No SoundModeListener attached");
  }
}

PlatformResult SoundManager::SetSoundModeChangeListener(
    SoundManagerSoundModeChangedListener* listener) {
  LoggerD("Enter");
  soundModeListener = listener;
  if (soundModeChangeListening) return PlatformResult(ErrorCode::NO_ERROR);

  int status = vconf_notify_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL,
                                        SoundManager::soundModeChangedCb, this);
  if (VCONF_OK == status) {
    soundModeChangeListening = true;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  LoggerE("SoundModeChangeListener no setted");
  return PlatformResult(ErrorCode::UNKNOWN_ERR,
                        "SoundModeChangeListener no setted");
}

PlatformResult SoundManager::UnsetSoundModeChangeListener() {
  LoggerD("Enter");
  soundModeListener = nullptr;
  if (!soundModeChangeListening) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int status = vconf_ignore_key_changed(VCONFKEY_SETAPPL_VIBRATION_STATUS_BOOL,
                                        SoundManager::soundModeChangedCb);
  if (VCONF_OK == status) {
    soundModeChangeListening = false;
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  LoggerE("SoundModeChangeListener no unsetted");
  return PlatformResult(ErrorCode::UNKNOWN_ERR,
                        "SoundModeChangeListener no unsetted");
}

PlatformResult SoundManager::SetVolumeChangeListener() {
  LoggerD("Enter");
  if (!is_volume_change_listener_) {
    int ret = sound_manager_set_volume_changed_cb(
        [](sound_type_e type, unsigned int value, void* ud) {
          return static_cast<SoundManager*>(ud)
              ->VolumeChangeCallback(type, value);
        },
        static_cast<void*>(this));

    if (ret != SOUND_MANAGER_ERROR_NONE) {
      LoggerE("Failed to set volume changed callback: error code: %d", ret);
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to set volume changed callback");
    }

    is_volume_change_listener_ = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::UnsetVolumeChangeListener() {
  LoggerD("Enter");
  if (!is_volume_change_listener_) {
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  int ret = sound_manager_unset_volume_changed_cb();
  if (ret != SOUND_MANAGER_ERROR_NONE) {
    LoggerE("Failed to unset volume changed callback");
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to unset volume changed callback");
  }

  is_volume_change_listener_ = false;

  return PlatformResult(ErrorCode::NO_ERROR);
}

void SoundManager::GetDeviceList(sound_device_mask_e mask, picojson::object& out) {
  LoggerD("Entered");

  sound_device_list_h device_list = nullptr;
  sound_device_h device = nullptr;

  picojson::value response = picojson::value(picojson::array());
  picojson::array& response_array = response.get<picojson::array>();

  int ret = sound_manager_get_current_device_list(mask, &device_list);
  if (SOUND_MANAGER_ERROR_NONE != ret && SOUND_MANAGER_ERROR_NO_DATA != ret) {
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device list failed"), &out);
    return;
  }

  while (!(ret = sound_manager_get_next_device(device_list, &device))) {
    picojson::value val = picojson::value(picojson::object());
    picojson::object& obj = val.get<picojson::object>();
    PlatformResult result = GetDeviceInfo(device, true, false, &obj);

    if (result.IsError()) {
      ReportError(result, &out);
      return;
    }
    response_array.push_back(val);
  }

  ReportSuccess(response, out);
}

PlatformResult SoundManager::GetDeviceInfo(sound_device_h device,
                                           bool is_connected,
                                           bool check_connection,
                                           picojson::object* obj) {
  LoggerD("Entered");

  //get id
  int id = 0;
  int ret = sound_manager_get_device_id(device, &id);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device id failed");
  }
  obj->insert(std::make_pair("id", picojson::value(static_cast<double>(id))));

  //get name
  char *name = nullptr;
  ret = sound_manager_get_device_name(device, &name);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device name failed");
  }
  obj->insert(std::make_pair("name", picojson::value(name)));

  //get type
  sound_device_type_e type = SOUND_DEVICE_BUILTIN_SPEAKER;
  ret = sound_manager_get_device_type(device, &type);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device type failed");
  }
  obj->insert(std::make_pair("device", picojson::value(SoundDeviceTypeToString(type))));

  //get direction
  sound_device_io_direction_e direction = SOUND_DEVICE_IO_DIRECTION_IN;
  ret = sound_manager_get_device_io_direction (device, &direction);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device direction failed");
  }
  obj->insert(std::make_pair("direction", picojson::value(SoundIOTypeToString(direction))));

  //get state
  sound_device_state_e state = SOUND_DEVICE_STATE_DEACTIVATED;
  ret = sound_manager_get_device_state(device, &state);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device state failed");
  }
  obj->insert(std::make_pair("isActivated", picojson::value(static_cast<bool>(state))));

  //get connection
  if (check_connection) {
    return IsDeviceConnected(type, direction, obj);
  }

  obj->insert(std::make_pair("isConnected", picojson::value(is_connected)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::IsDeviceConnected(sound_device_type_e type,
                                               sound_device_io_direction_e direction,
                                               picojson::object* obj) {
  LoggerD("Entered");

  sound_device_mask_e mask = SOUND_DEVICE_ALL_MASK;
  switch (direction) {
    case SOUND_DEVICE_IO_DIRECTION_IN:
      mask = SOUND_DEVICE_IO_DIRECTION_IN_MASK;
      break;
    case SOUND_DEVICE_IO_DIRECTION_OUT:
      mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
      break;
    case SOUND_DEVICE_IO_DIRECTION_BOTH:
      mask = SOUND_DEVICE_IO_DIRECTION_BOTH_MASK;
      break;
    default:
      LoggerD("Invalid IOType (%d)", direction);
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Invalid IO type");
  }

  sound_device_list_h device_list = nullptr;
  sound_device_h device = nullptr;
  sound_device_type_e device_type = SOUND_DEVICE_BUILTIN_SPEAKER;

  int ret = sound_manager_get_current_device_list(mask, &device_list);
  if (SOUND_MANAGER_ERROR_NONE != ret) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device list failed");
  }

  while (!(ret = sound_manager_get_next_device(device_list, &device))) {
    ret = sound_manager_get_device_type(device, &device_type);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Getting device type failed");
    }

    if (type == device_type) {
      obj->insert(std::make_pair("isConnected", picojson::value(true)));
      return PlatformResult(ErrorCode::NO_ERROR);
    }
  }

  obj->insert(std::make_pair("isConnected", picojson::value(false)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void SoundManager::DeviceChangeCB(sound_device_h device, bool is_connected, bool check_connection) {
  LoggerD("Entered");

  picojson::value response = picojson::value(picojson::object());
  picojson::object& response_obj = response.get<picojson::object>();

  PlatformResult result = GetDeviceInfo(device, is_connected, check_connection, &response_obj);

  if (result.IsSuccess()) {
    response_obj.insert(std::make_pair(
        "listenerId", picojson::value("SoundDeviceStateChangeCallback")));

    auto call_response = [this, response]()->void {
      Instance::PostMessage(&instance_, response.serialize().c_str());
    };

    TaskQueue::GetInstance().Async(call_response);
  }
}

void DeviceConnectionChangeCB(sound_device_h device, bool is_connected, void *user_data) {
  LoggerD("Entered");
  SoundManager* h = static_cast<SoundManager*>(user_data);
  h->DeviceChangeCB(device, is_connected, false);
}

void DeviceActivationChangeCB(sound_device_h device, sound_device_changed_info_e changed_info,
                              void *user_data) {
  LoggerD("Entered");

  if (SOUND_DEVICE_CHANGED_INFO_STATE == changed_info) {
    SoundManager* h = static_cast<SoundManager*>(user_data);
    h->DeviceChangeCB(device, false, true);
  }
}

PlatformResult SoundManager::AddDeviceStateChangeListener() {
  LoggerD("Entered");

  int ret = SOUND_MANAGER_ERROR_NONE;
  sound_device_mask_e mask = SOUND_DEVICE_ALL_MASK;

  if (!sound_device_change_listener_) {
    ret = sound_manager_set_device_connected_cb(mask, DeviceConnectionChangeCB, this);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Setting connection listener failed");
    }

    ret = sound_manager_set_device_information_changed_cb(mask, DeviceActivationChangeCB, this);
    if (SOUND_MANAGER_ERROR_NONE != ret) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Setting information listener failed");
    }

    sound_device_change_listener_ = true;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult SoundManager::RemoveDeviceStateChangeListener() {
  LoggerD("Entered");

  if (sound_device_change_listener_) {
    if (SOUND_MANAGER_ERROR_NONE != sound_manager_unset_device_connected_cb()) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unsetting information listener failed");
    }

    if (SOUND_MANAGER_ERROR_NONE != sound_manager_unset_device_information_changed_cb()) {
      return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unsetting information listener failed");
    }

    sound_device_change_listener_ = false;
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

}  // namespace sound
}  // namespace extension
