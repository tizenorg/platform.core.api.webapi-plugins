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

#include "bluetooth_adapter.h"

#include <algorithm>
#include <memory>

#include <pcrecpp.h>
#include <system_info.h>
#include <bluetooth.h>

#include "common/converter.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"
#include "common/extension.h"
#include "common/task-queue.h"

#include "bluetooth_class.h"
#include "bluetooth_device.h"
#include "bluetooth_privilege.h"
#include "bluetooth_socket.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;
using namespace common::tools;

namespace {
const std::string kAction = "action";
const std::string kData = "data";
const std::string kName = "name";
//adapter
const std::string kAdapterPowered = "powered";
const std::string kAdapterVisible = "visible";
//AdapterChangeCallback
const std::string kOnStateChanged = "onstatechanged";
const std::string kOnNameChanged = "onnamechanged";
const std::string kOnVisibilityChanged = "onvisibilitychanged";
const std::string kAdapterChangeCallbackEvent = "BluetoothAdapterChangeCallback";
// BluetoothProfileHandler
const std::string kBluetoothProfileHealth = "HEALTH";
const std::string kFeatureBluetoothHealth = "tizen.org/feature/network.bluetooth.health";
//DiscoverDevicesCallback
const std::string kOnDiscoverStarted = "onstarted";
const std::string kOnDiscoverFound = "ondevicefound";
const std::string kOnDiscoverDisappeared = "ondevicedisappeared";
const std::string kOnDiscoverFinished = "onfinished";
const std::string kAdapterDiscoverSuccessEvent = "BluetoothDiscoverDevicesSuccessCallback";
const std::string kAdapterDiscoverErrorEvent = "BluetoothDiscoverDevicesErrorCallback";
//device
const std::string kDeviceAddress = "address";
const unsigned short kTimeout = 180;
}

static bool IsValidAddress(const std::string& address) {
    static pcrecpp::RE re("(([0-9a-zA-Z]+):)+([0-9a-zA-Z]+)");
    static  std::string compare_address = "00:12:47:08:9A:A6";

    if (!re.FullMatch(address)) {
        LoggerE("Invalid address");
        return false;
    }
    if (address.size() != compare_address.size()) {
        LoggerE("Invalid size");
        return false;
    }
    return true;
}

static bool IsValidUUID(const std::string& uuid) {
    static pcrecpp::RE re("(([0-9a-zA-Z]+)-)+([0-9a-zA-Z]+)");
    static std::string compare_uuid = "00001101-0000-1000-8000-00805F9B34FB";

    if (!re.FullMatch(uuid)) {
        LoggerE("Invalid UUID: %s", uuid.c_str());
        return false;
    }

    if (uuid.size() != compare_uuid.size()) {
        LoggerE("Invalid size: %s", uuid.c_str());
        return false;
    }

    return true;
}

void BluetoothAdapter::StateChangedCB(int result, bt_adapter_state_e state, void *user_data) {
    LoggerD("Entered");

    BluetoothAdapter* adapter = static_cast<BluetoothAdapter*>(user_data);
    if (!adapter) {
        LoggerD("User data is NULL");
        return;
    }

    const bool powered = BT_ADAPTER_ENABLED == state;
    bool previous_powered = adapter->is_powered_;
    adapter->is_powered_ = powered;

    if (powered) {
        //update visible state if bluetooth device has been turned on
        adapter->is_visible_ = adapter->get_visible();
    }

    if (previous_powered != powered && BT_ERROR_NONE == result) {
        picojson::value value = picojson::value(picojson::object());
        picojson::object* data_obj = &value.get<picojson::object>();

        data_obj->insert(std::make_pair(kAction, picojson::value(kOnStateChanged)));
        data_obj->insert(std::make_pair(kAdapterPowered, picojson::value(powered)));

        util::FireEvent(kAdapterChangeCallbackEvent, value);
    }

    if (adapter->user_request_list_[SET_POWERED]) {
        if (adapter->requested_powered_ != powered) {
            return;
        }

      PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
      switch(result) {
          case BT_ERROR_NONE:
          case BT_ERROR_ALREADY_DONE:
          case BT_ERROR_NOT_ENABLED:
              //do nothing
              break;
          case BT_ERROR_NOW_IN_PROGRESS:
              ret = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is busy");
              break;
          default:
              ret = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
      }

      util::AsyncResponse(adapter->user_request_callback_[SET_POWERED], ret);
      adapter->user_request_list_[SET_POWERED] = false;
  }
}

void BluetoothAdapter::NameChangedCB(char *name, void *user_data) {
    LoggerD("Entered");

    BluetoothAdapter* adapter = static_cast<BluetoothAdapter*>(user_data);
    if (!adapter) {
        LoggerD("User data is NULL");
        return;
    }

    picojson::value value = picojson::value(picojson::object());
    picojson::object* data_obj = &value.get<picojson::object>();

    data_obj->insert(std::make_pair(kAction, picojson::value(kOnNameChanged)));
    data_obj->insert(std::make_pair(kName, picojson::value(name)));

    util::FireEvent(kAdapterChangeCallbackEvent, value);

    if (adapter->user_request_list_[SET_NAME] && name == adapter->requested_name_) {
        std::shared_ptr<picojson::value> response =
                std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
        util::AsyncResponse(adapter->user_request_callback_[SET_NAME], response);
        adapter->user_request_list_[SET_NAME] = false;
    }
}

void BluetoothAdapter::VisibilityChangedCB(int result, bt_adapter_visibility_mode_e mode, void *user_data) {
    LoggerD("Entered");

    BluetoothAdapter* adapter = static_cast<BluetoothAdapter*>(user_data);
    if (!adapter) {
        LoggerD("User data is NULL");
        return;
    }

    bool visible = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE != mode;
    bool previous_visible = adapter->is_visible_;
    adapter->is_visible_ = visible;

    if (previous_visible != visible) {
        picojson::value value = picojson::value(picojson::object());
        picojson::object* data_obj = &value.get<picojson::object>();

        data_obj->insert(std::make_pair(kAction, picojson::value(kOnVisibilityChanged)));
        data_obj->insert(std::make_pair(kAdapterVisible, picojson::value(visible)));

        util::FireEvent(kAdapterChangeCallbackEvent, value);
    }

    if (adapter->user_request_list_[SET_VISIBLE] && adapter->requested_visibility_ == mode) {
        PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);

        if (BT_ERROR_NONE != result) {
            ret = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
        }

        util::AsyncResponse(adapter->user_request_callback_[SET_VISIBLE], ret);
        adapter->user_request_list_[SET_VISIBLE] = false;
    }
}

static bool ForeachBondedDevicesCB(bt_device_info_s *device_info, void *user_data)
{
    LoggerD("Entered");
    if (nullptr == user_data) {
        LoggerD("user data is NULL.");
        return false;
    }

    if (nullptr == device_info) {
        LoggerD("Device info is not valid.");
        return false;
    }

    picojson::array* array = static_cast<picojson::array*>(user_data);
    for (auto iter = array->begin(); iter != array->end(); iter++) {
        if (!strcmp(device_info->remote_address, ((*iter).get<picojson::object>())
                    .find(kDeviceAddress)->second.get<std::string>().c_str())) {
            BluetoothDevice::ToJson(device_info, &iter->get<picojson::object>());
            return true;
        }
    }

    array->push_back(picojson::value(picojson::object()));

    BluetoothDevice::ToJson(device_info, &array->back().get<picojson::object>());
    return true;
}

void BluetoothAdapter::DiscoveryStateChangedCB(
        int result,
        bt_adapter_device_discovery_state_e discovery_state,
        bt_adapter_device_discovery_info_s *discovery_info,
        void *user_data)
{
    LoggerD("Entered");

    BluetoothAdapter* adapter = static_cast<BluetoothAdapter*>(user_data);
    if (!adapter) {
        LoggerD("User data is NULL");
        return;
    }

    picojson::value value = picojson::value(picojson::object());
    picojson::object* data_obj = &value.get<picojson::object>();

    switch(discovery_state) {
        case BT_ADAPTER_DEVICE_DISCOVERY_STARTED: {
            if (adapter->user_request_list_[DISCOVER_DEVICES]) {
                if (BT_ERROR_NONE == result) {
                    //store addresses of previously found devices into disappeared_addresses
                    adapter->disappeared_addresses_ = adapter->discovered_addresses_;
                    adapter->discovered_addresses_.clear();
                    adapter->discovered_devices_.clear();

                    data_obj->insert(std::make_pair(kAction, picojson::value(kOnDiscoverStarted)));
                    util::FireEvent(kAdapterDiscoverSuccessEvent, value);
                } else {
                    util::ReportError(UnknownException("Unknown error"), *data_obj);
                    util::FireEvent(kAdapterDiscoverErrorEvent, value);
                    adapter->user_request_list_[DISCOVER_DEVICES] = false;
                }
            }
            break;
        }
        case BT_ADAPTER_DEVICE_DISCOVERY_FINISHED: {
            if (BT_ERROR_NONE == result || BT_ERROR_CANCELLED == result) {
                if (adapter->user_request_list_[DISCOVER_DEVICES]) {
                    data_obj->insert(std::make_pair(kAction, picojson::value(kOnDiscoverFinished)));

                    for (auto it : adapter->disappeared_addresses_) {
                        picojson::value disapeared_val = picojson::value(picojson::object());
                        picojson::object* disapeared_obj = &disapeared_val.get<picojson::object>();

                        disapeared_obj->insert(std::make_pair(kAction,
                                               picojson::value(kOnDiscoverDisappeared)));
                        disapeared_obj->insert(std::make_pair(kData, picojson::value(it)));

                        util::FireEvent(kAdapterDiscoverSuccessEvent, disapeared_val);
                    }

                    data_obj->insert(std::make_pair(kData,
                            picojson::value(adapter->discovered_devices_)));
                    util::FireEvent(kAdapterDiscoverSuccessEvent, value);

                    adapter->user_request_list_[DISCOVER_DEVICES] = false;
                }

                if (adapter->user_request_list_[STOP_DISCOVERY]) {
                    std::shared_ptr<picojson::value> response =
                            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

                    util::ReportSuccess(response->get<picojson::object>());
                    util::AsyncResponse(
                            adapter->user_request_callback_[STOP_DISCOVERY], response);

                    adapter->user_request_list_[STOP_DISCOVERY] = false;
                }
            }
            break;
        }
        case BT_ADAPTER_DEVICE_DISCOVERY_FOUND: {
            if (adapter->user_request_list_[DISCOVER_DEVICES]) {
                if (BT_ERROR_NONE == result &&
                        adapter->discovered_addresses_.insert(
                                discovery_info->remote_address).second) {
                    adapter->disappeared_addresses_.erase(discovery_info->remote_address);

                    data_obj->insert(std::make_pair(kAction, picojson::value(kOnDiscoverFound)));
                    picojson::value& data = data_obj->insert(std::make_pair(kData,
                            picojson::value(picojson::object()))).first->second;

                    BluetoothDevice::ToJson(discovery_info, &data.get<picojson::object>());
                    adapter->discovered_devices_.push_back(data);

                    util::FireEvent(kAdapterDiscoverSuccessEvent, value);
                }
            }
            break;
        }
        default:
            LoggerD("Unknown state");
            break;
    }
}

BluetoothAdapter::BluetoothAdapter() :
    is_visible_(false),
    is_powered_(false),
    is_initialized_(false)
{
    LoggerD("Entered");
    if (BT_ERROR_NONE == bt_initialize()) {
        LoggerD("Bluetooth service is initialized.");
        is_initialized_ = true;

        int ret = BT_ERROR_NONE;
        ret |= bt_adapter_set_device_discovery_state_changed_cb(DiscoveryStateChangedCB, this);
        ret |= bt_adapter_set_state_changed_cb(StateChangedCB, this);
        ret |= bt_adapter_set_name_changed_cb(NameChangedCB, this);
        ret |= bt_adapter_set_visibility_mode_changed_cb(VisibilityChangedCB, this);

        if (BT_ERROR_NONE != ret) {
            LoggerE("Setting listeners function failed.");
        }
    } else {
        LoggerE("Bluetooth service initialization failed.");
    }

    bt_adapter_state_e state;
    if (BT_ERROR_NONE == bt_adapter_get_state(&state)) {
        is_powered_ = BT_ADAPTER_ENABLED == state;
    }

    bt_adapter_visibility_mode_e mode;
    if (BT_ERROR_NONE == bt_adapter_get_visibility(&mode, nullptr)) {
        is_visible_ = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE != mode;
    }
}

BluetoothAdapter::~BluetoothAdapter() {
    bt_socket_unset_data_received_cb();
    bt_socket_unset_connection_state_changed_cb();

    for (auto it : connected_sockets_) {
        bt_socket_disconnect_rfcomm(it);
    }

    for (auto it : registered_uuids_) {
        bt_socket_destroy_rfcomm(it.second.first);
    }

    bt_adapter_unset_state_changed_cb();
    bt_adapter_unset_name_changed_cb();
    bt_adapter_unset_visibility_mode_changed_cb();
    bt_adapter_unset_device_discovery_state_changed_cb();

    if (is_initialized_) {
        if (BT_ERROR_NONE == bt_deinitialize()) {
            LoggerD("Bluetooth service is deinitialized.");
        } else {
            LoggerE("Bluetooth service deinitialization failed.");
        }
    }
}

BluetoothAdapter& BluetoothAdapter::GetInstance() {
    static BluetoothAdapter instance;
    return instance;
}

std::string BluetoothAdapter::get_name() const {
    char* name = nullptr;
    std::string str_name = "";
    if (BT_ERROR_NONE == bt_adapter_get_name(&name)) {
        if (name) {
            str_name = name;
            free(name);
        }
    }

    return str_name;
}

bool BluetoothAdapter::get_visible() const {
    bt_adapter_visibility_mode_e mode;

    if (BT_ERROR_NONE == bt_adapter_get_visibility(&mode, NULL)) {
        return mode != BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
    }

    return false;
}

void BluetoothAdapter::set_visible(bool visible) {
    is_visible_ = visible;
}

bool BluetoothAdapter::get_powered() {
    return is_powered_;
}

void BluetoothAdapter::set_powered(bool powered) {
    is_powered_ = powered;
}

bool BluetoothAdapter::is_initialized() const {
    return is_initialized_;
}

void BluetoothAdapter::SetName(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothAdmin);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);
    const auto name = FromJson<std::string>(args, "name");

    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);
    if (!this->is_initialized()) {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Bluetooth service is not initialized.");
        util::AsyncResponse(callback_handle, result);
        return;
    }

    if (this->get_powered()) {
        if (get_name() == name) {
            util::AsyncResponse(callback_handle, result);
            return;
        }

        if (this->user_request_list_[SET_NAME]) {
            result = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Already requested");
            util::AsyncResponse(callback_handle, result);
            return;
        }

        this->user_request_list_[SET_NAME] = true;
        this->user_request_callback_[SET_NAME] = callback_handle;

        int ret = bt_adapter_set_name(name.c_str());
        switch(ret) {
            case BT_ERROR_NONE:
                //bt_adapter_name_changed_cb() will be invoked
                //if this function returns #BT_ERROR_NONE
                this->requested_name_ = name;
                break;
            case BT_ERROR_INVALID_PARAMETER:
                result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid value");
                break;
            default:
               result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
        }
    } else {
        result = PlatformResult(
                ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
    }

    if (result.IsError()) {
        this->user_request_list_[SET_NAME] = false;
    }

    util::AsyncResponse(callback_handle, result);
}

void BluetoothAdapter::SetPowered(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothAdmin);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);
    const auto new_powered = FromJson<bool>(args, "powered");

    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);

    if (!this->is_initialized()) {
        ret = PlatformResult(ErrorCode::UNKNOWN_ERR, "Bluetooth service is not initialized.");
    }

    if (ret.IsSuccess() && this->user_request_list_[SET_POWERED]) {
        ret = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Already requested");
    }

    bool cur_powered = this->get_powered();

    if (ret.IsSuccess() && new_powered != cur_powered) {
        this->requested_powered_ = new_powered;
        this->user_request_list_[SET_POWERED] = true;
        this->user_request_callback_[SET_POWERED] = callback_handle;

        if (new_powered) {
            bt_adapter_enable();
        } else {
            bt_adapter_disable();
        }
    }

    util::AsyncResponse(callback_handle, ret);
}

void BluetoothAdapter::SetVisible(const picojson::value& data, picojson::object& out)
{
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothManager);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);
    const auto visible = FromJson<bool>(args, "visible");

    unsigned short timeout = kTimeout;
    if (visible) {
        timeout = static_cast<unsigned short>(FromJson<double>(args, "timeout"));
    }

    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

    if (!this->is_initialized()) {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Bluetooth service is not initialized.");
    }

    if (result.IsSuccess() && this->user_request_list_[SET_VISIBLE]) {
        result = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Already requested");
    }

    if (result.IsSuccess() && this->get_powered()) {
        bt_adapter_visibility_mode_e mode = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
        if (visible) {
            if (0 == timeout) {
                mode = BT_ADAPTER_VISIBILITY_MODE_GENERAL_DISCOVERABLE;
            } else {
                mode = BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE;
            }
        }

        bt_adapter_visibility_mode_e current = BT_ADAPTER_VISIBILITY_MODE_NON_DISCOVERABLE;
        int time = 0;
        if (BT_ERROR_NONE != bt_adapter_get_visibility(&current , &time)) {
          result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
          util::AsyncResponse(callback_handle, result);
          return;
        }

        if (mode == current) {
            if (BT_ADAPTER_VISIBILITY_MODE_LIMITED_DISCOVERABLE != mode ||
                    (unsigned int)time != timeout) {
                util::AsyncResponse(callback_handle, result);
                return;
            }
        }

        this->requested_visibility_ = mode;
        this->user_request_list_[SET_VISIBLE] = true;
        this->user_request_callback_[SET_VISIBLE] = callback_handle;
        int ret = bt_adapter_set_visibility(mode, timeout);

        switch(ret) {
            case BT_ERROR_NONE:
                //bt_adapter_visibility_mode_changed_cb() will be invoked
                //if this function returns #BT_ERROR_NONE
                break;
            case BT_ERROR_INVALID_PARAMETER:
                result = PlatformResult(ErrorCode::INVALID_VALUES_ERR, "Invalid value");
                break;
            default:
                result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
        }
    } else if (result.IsSuccess()){
        result = PlatformResult(
                ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
    }

    util::AsyncResponse(callback_handle, result);
}

void BluetoothAdapter::DiscoverDevices(const picojson::value& /* data */, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

    if (!is_initialized_) {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Bluetooth service is not initialized.");
    }

    if (result.IsSuccess() && this->user_request_list_[DISCOVER_DEVICES]) {
        result = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Already requested");
    }

    if (result.IsSuccess() && !get_powered()) {
        result = PlatformResult(
                ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
    }

    if (result.IsSuccess()) {
        this->user_request_list_[DISCOVER_DEVICES] = true;
        bt_adapter_start_device_discovery();
    } else {
        std::shared_ptr<picojson::value> response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

        ReportError(result, &response->get<picojson::object>());
        TaskQueue::GetInstance().Async<picojson::value>([](const std::shared_ptr<picojson::value>& result) {
            util::FireEvent(kAdapterDiscoverErrorEvent, result);
        }, response);
    }
}

void BluetoothAdapter::StopDiscovery(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);

    PlatformResult result = PlatformResult(ErrorCode::NO_ERROR);

    if (!this->is_initialized()) {
        result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Bluetooth service is not initialized.");
    }

    if (result.IsSuccess() && this->user_request_list_[STOP_DISCOVERY]) {
        result = PlatformResult(ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Already requested");
    }

    if (result.IsSuccess() && this->get_powered()) {
        bool is_discovering = false;
        bt_adapter_is_discovering(&is_discovering);

        if (!is_discovering) {
            util::AsyncResponse(callback_handle, result);
            return;
        }

        this->user_request_list_[STOP_DISCOVERY] = true;
        this->user_request_callback_[STOP_DISCOVERY] = callback_handle;
        int ret = bt_adapter_stop_device_discovery();
        switch(ret) {
            case BT_ERROR_NONE: {
                //This function invokes bt_adapter_device_discovery_state_changed_cb().
                break;
            }
            default: {
                this->user_request_list_[STOP_DISCOVERY] = false;
                result = PlatformResult(ErrorCode::UNKNOWN_ERR, "Unknown exception");
            }
        }
    } else if (result.IsSuccess()) {
        result = PlatformResult(
                ErrorCode::SERVICE_NOT_AVAILABLE_ERR, "Bluetooth device is turned off");
    }

    if (result.IsError()) {
        util::AsyncResponse(callback_handle, result);
    }
}

void BluetoothAdapter::GetKnownDevices(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);

    auto get_known_devices = [this](const std::shared_ptr<picojson::value>& response) -> void {
        try {
            if (!this->is_initialized()) {
                throw UnknownException("Bluetooth service is not initialized.");
            }
            if (this->get_powered()) {
                picojson::object& response_obj = response->get<picojson::object>();
                picojson::value result = picojson::value(picojson::object());
                picojson::object& result_obj = result.get<picojson::object>();
                picojson::array& array = result_obj.insert(
                    std::make_pair("devices", picojson::value(
                            picojson::array()))).first->second.get<picojson::array>();

                array = discovered_devices_;

                if (BT_ERROR_NONE == bt_adapter_foreach_bonded_device(
                        ForeachBondedDevicesCB, &array)) {
                    util::ReportSuccess(result, response_obj);
                } else {
                    throw UnknownException("Unknown exception");
                }
            } else {
                throw ServiceNotAvailableException("Bluetooth device is turned off");
            }
        } catch (const PlatformException& err) {
            util::ReportError(err, response->get<picojson::object>());
        }
    };
    auto get_known_devices_response = [callback_handle](
            const std::shared_ptr<picojson::value>& response) -> void {
        util::SyncResponse(callback_handle, response);
    };

    TaskQueue::GetInstance().Queue<picojson::value>(
        get_known_devices,
        get_known_devices_response,
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));

    util::ReportSuccess(out);
}

void BluetoothAdapter::GetDevice(const picojson::value& data, picojson::object&  out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);

    const auto& address = FromJson<std::string>(args, "address");

    auto get_device = [this, address](const std::shared_ptr<picojson::value>& response) -> void {
        try {
            if (!IsValidAddress(address)) {
                throw NotFoundException("Wrong address");
            }

            if (!this->is_initialized()) {
                throw UnknownException("Bluetooth service is not initialized.");
            }
            if (this->get_powered()) {
                picojson::object& response_obj = response->get<picojson::object>();
                bt_device_info_s *info = nullptr;

                if (bt_adapter_get_bonded_device_info(address.c_str(), &info) == BT_ERROR_NONE &&
                                info != nullptr) {
                    picojson::value result = picojson::value(picojson::object());
                    picojson::object& result_obj = result.get<picojson::object>();

                    BluetoothDevice::ToJson(info, &result_obj);
                    util::ReportSuccess(result, response_obj);
                    bt_adapter_free_device_info(info);
                    return;
                }

                auto is_address = discovered_addresses_.find(address);
                if (is_address != discovered_addresses_.end()) {
                    for (auto iter = discovered_devices_.begin();
                            iter != discovered_devices_.end(); iter++) {
                        if (!strcmp(address.c_str(), ((*iter).get<picojson::object>())
                                    .find(kDeviceAddress)->second.get<std::string>().c_str())) {
                            util::ReportSuccess(*iter, response_obj);
                            return;
                        }
                    }
                } else {
                    throw NotFoundException("There is no device with the given address");
                }
            } else {
                throw ServiceNotAvailableException("Bluetooth device is turned off");
            }
        } catch (const PlatformException& err) {
            util::ReportError(err, response->get<picojson::object>());
        }
    };

    auto get_device_response = [callback_handle](
            const std::shared_ptr<picojson::value>& response) -> void {
        util::SyncResponse(callback_handle, response);
    };

    TaskQueue::GetInstance().Queue<picojson::value>(
        get_device,
        get_device_response,
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));

    util::ReportSuccess(out);
}

class BondingHandler {
public:
    BondingHandler(double callback_handle, const std::string& address) :
            callback_handle_(callback_handle), address_(address) {}

    void set_address(const std::string& address) {
        address_ = address;
    }

    const std::string& address() const {
        return address_;
    }

    void Invoke(const std::shared_ptr<picojson::value>& response) {
        LoggerD("Entered");
        util::AsyncResponse(callback_handle_, response);
    }

private:
    double callback_handle_;
    std::string address_;
};

void BluetoothAdapter::CreateBonding(const picojson::value& data, picojson::object& out)
{
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);

    const auto& address = FromJson<std::string>(args, "address");

    auto create_bonding = [address, callback_handle, this]() -> void {
        try {
            if(!IsValidAddress(address)) {
                throw InvalidValuesException("Wrong address");
            }
            if (!this->is_initialized()) {
                throw UnknownException("Bluetooth service is not initialized.");
            }

            if (this->get_powered()) {

                auto bond_create_callback = [](int callback_result,
                                             bt_device_info_s *device_info,
                                             void *user_data) {
                    LoggerD("bond_create_callback");

                    BondingHandler* handler = static_cast<BondingHandler*>(user_data);
                    if (!handler) {
                        LoggerW("user_data is nullptr");
                        return;
                    }
                    if (!device_info) {
                        LoggerW("device_info is nullptr");
                        return;
                    }

                    if (!strcmp(handler->address().c_str(), device_info->remote_address)) {  // requested event
                        try {
                            if (BT_ERROR_NONE == callback_result && nullptr != device_info ) {
                                std::shared_ptr<picojson::value> response =
                                        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
                                picojson::object& response_obj = response->get<picojson::object>();
                                picojson::value result = picojson::value(picojson::object());
                                picojson::object& result_obj = result.get<picojson::object>();

                                BluetoothDevice::ToJson(device_info, &result_obj);
                                util::ReportSuccess(result, response_obj);
                                handler->Invoke(response);
                            } else if (BT_ERROR_REMOTE_DEVICE_NOT_FOUND == callback_result) {
                                LoggerE("Not found");
                                throw ServiceNotAvailableException("Not found");
                            } else {
                                LoggerE("Unknown exception");
                                throw UnknownException("Unknown exception");
                            }
                        } catch (const PlatformException& err) {
                            std::shared_ptr<picojson::value> response =
                                    std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
                            util::ReportError(err, response->get<picojson::object>());
                            handler->Invoke(response);
                        }
                        delete handler;
                        bt_device_unset_bond_created_cb();
                    } else {  // unexpected event
                        LoggerD("An unexpected bonding detected");
                    }
                };

                BondingHandler* handler = new BondingHandler(callback_handle, address);
                bt_device_set_bond_created_cb(bond_create_callback, handler);
                int ret = bt_device_create_bond(address.c_str());

                switch(ret) {
                    case BT_ERROR_NONE:
                    {
                        LoggerD("bt_device_create_bond() succeeded");
                        break;
                    }
                    case BT_ERROR_INVALID_PARAMETER:
                    {
                        LoggerE("Not found");
                        bt_device_unset_bond_created_cb();
                        delete handler;
                        throw InvalidValuesException("Invalid value");
                    }
                    default:
                    {
                        LoggerE("Unknown exception");
                        bt_device_unset_bond_created_cb();
                        delete handler;
                        throw UnknownException("Unknown exception");
                    }
                }
            } else {   // Not powered
                LoggerE("Bluetooth device is turned off");
                throw ServiceNotAvailableException("Bluetooth device is turned off");
            }
        } catch (const PlatformException& err) {
            std::shared_ptr<picojson::value> response =
                    std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
            util::ReportError(err, response->get<picojson::object>());
            util::AsyncResponse(callback_handle, response);
        }
    };
    TaskQueue::GetInstance().Queue(create_bonding);
    util::ReportSuccess(out);
}

void BluetoothAdapter::DestroyBonding(const picojson::value& data, picojson::object& out)
{
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothGap);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);

    const auto& address = FromJson<std::string>(args, "address");

    auto destroy_bonding = [address, callback_handle, this]() -> void {
        try {
            if(!IsValidAddress(address)) {
                throw InvalidValuesException("Wrong address");
            }
            if (!this->is_initialized()) {
                throw UnknownException("Bluetooth service is not initialized.");
            }

            if (this->get_powered()) {
                bt_device_info_s *device_info = nullptr;
                int ret = bt_adapter_get_bonded_device_info(address.c_str(), &device_info);

                if (BT_ERROR_NONE != ret || nullptr == device_info) {
                    LoggerD("There is no bonding");
                    throw NotFoundException("Not found");
                } else {
                    bt_adapter_free_device_info(device_info);

                    auto bond_destroy_callback = [](int callback_result,
                                                    char *remote_address,
                                                    void *user_data) {
                        LoggerD("bond_destroy_callback");

                        BondingHandler* handler = static_cast<BondingHandler*>(user_data);
                        if (!handler) {
                            LoggerW("user_data is nullptr");
                            return;
                        }

                        if (!strcmp(handler->address().c_str(), remote_address)) {  // requested event
                            try {
                                if (BT_ERROR_NONE == callback_result) {
                                    std::shared_ptr<picojson::value> response =
                                            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
                                    util::ReportSuccess(response->get<picojson::object>());
                                    handler->Invoke(response);
                                } else {
                                    LoggerE("Unknown exception");
                                    throw UnknownException("Unknown exception");
                                }
                            } catch (const PlatformException& err) {
                                std::shared_ptr<picojson::value> response =
                                        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
                                util::ReportError(err, response->get<picojson::object>());
                                handler->Invoke(response);
                            }
                            delete handler;
                            bt_device_unset_bond_destroyed_cb();
                        } else {  // unexpected event
                            LoggerD("An unexpected bonding detected");
                        }
                    };

                    BondingHandler* handler = new BondingHandler(callback_handle, address);
                    bt_device_set_bond_destroyed_cb(bond_destroy_callback, handler);

                    int ret = bt_device_destroy_bond(address.c_str());

                    switch(ret) {
                        case BT_ERROR_NONE:
                        {
                            LoggerD("bt_device_destroy_bond() succeeded");
                            break;
                        }
                        case BT_ERROR_INVALID_PARAMETER:
                        {
                            LoggerE("Not found");
                            bt_device_unset_bond_destroyed_cb();
                            delete handler;
                            throw InvalidValuesException("Invalid value");
                        }
                        default:
                        {
                            LoggerE("Unknown exception");
                            bt_device_unset_bond_destroyed_cb();
                            delete handler;
                            throw UnknownException("Unknown exception");
                        }
                    }
                }
            } else {   // Not powered
                LoggerE("Bluetooth device is turned off");
                throw ServiceNotAvailableException("Bluetooth device is turned off");
            }
        } catch (const PlatformException& err) {
            std::shared_ptr<picojson::value> response =
                    std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
            util::ReportError(err, response->get<picojson::object>());
            util::AsyncResponse(callback_handle, response);
        }
    };
    TaskQueue::GetInstance().Queue(destroy_bonding);
    util::ReportSuccess(out);
}

void BluetoothAdapter::RegisterRFCOMMServiceByUUID(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    util::CheckAccess(Privilege::kBluetoothSpp);

    const auto callback_handle = util::GetAsyncCallbackHandle(data);
    const auto& args = util::GetArguments(data);

    const auto& uuid = FromJson<std::string>(args, "uuid");
    const auto& name = FromJson<std::string>(args, "name");

    auto rfcomm = [this, uuid, name](const std::shared_ptr<picojson::value>& response) -> void {
        try {
            if (!this->is_initialized()) {
                throw UnknownException("Bluetooth service is not initialized.");
            }

            if (!IsValidUUID(uuid)) {
                throw InvalidValuesException("Wrong UUID");
            }

            if (this->get_powered()) {
                bool is_registered = false;
                int ret = bt_adapter_is_service_used(uuid.c_str(), &is_registered);

                if (BT_ERROR_NONE == ret && is_registered) {
                    throw InvalidValuesException("Already registered");
                }

                int socket = -1;
                ret = bt_socket_create_rfcomm(uuid.c_str(), &socket);

                switch (ret) {
                    case BT_ERROR_NONE: {
                        int ret_in = bt_socket_listen_and_accept_rfcomm(socket, 0);
                        switch(ret_in) {
                            case BT_ERROR_NONE: {
                                LoggerD("bt_socket_listen() succeeded");
                                bt_socket_set_connection_state_changed_cb(OnSocketConnected, this);

                                registered_uuids_.insert(std::make_pair(uuid,
                                        std::make_pair(socket, false)));

                                util::ReportSuccess(response->get<picojson::object>());
                                break;
                            }

                            case BT_ERROR_INVALID_PARAMETER: {
                                throw InvalidValuesException("Invalid value");
                                break;
                            }

                            default: {
                                throw UnknownException("Unknown error");
                                break;
                            }
                        }
                        break;
                    }

                    case BT_ERROR_INVALID_PARAMETER:
                        throw InvalidValuesException("Invalid value");
                        break;

                    default:
                        throw UnknownException("Unknown error");
                        break;
                }
            } else {
                throw ServiceNotAvailableException("Bluetooth device is turned off");
            }
        } catch (const PlatformException& err) {
            util::ReportError(err, response->get<picojson::object>());
        }
    };

    auto rfcomm_response = [callback_handle](const std::shared_ptr<picojson::value>& response) -> void {
        util::SyncResponse(callback_handle, response);
    };

    TaskQueue::GetInstance().Queue<picojson::value>(rfcomm, rfcomm_response,
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));

    util::ReportSuccess(out);
}

void BluetoothAdapter::UnregisterUUID(const std::string& uuid, int callback_handle) {
    LoggerD("Entered");

    if (!IsValidUUID(uuid)) {
        throw InvalidValuesException("Wrong UUID");
    }

    std::shared_ptr<picojson::value> response =
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

    try {
        if (is_powered_) {
            auto iter = registered_uuids_.find(uuid);
            if (iter != registered_uuids_.end()) {
                if (BT_ERROR_NONE == bt_socket_destroy_rfcomm(iter->second.first)) {
                    registered_uuids_.erase(iter);
                } else {
                    throw UnknownException("Unknown error");
                }
            }

            if (registered_uuids_.size() == 0 &&
                connection_requests_.size() == 0 &&
                connected_sockets_.size() == 0) {
                bt_socket_unset_connection_state_changed_cb();
            }
        } else {
            throw ServiceNotAvailableException("Bluetooth device is turned off");
        }

        util::ReportSuccess(response->get<picojson::object>());
    } catch (const PlatformException& err) {
        util::ReportError(err, response->get<picojson::object>());
    }

    util::AsyncResponse(callback_handle, response);
}

void BluetoothAdapter::GetBluetoothProfileHandler(const picojson::value& data,
                                                  picojson::object& out) {
    LoggerD("Entered");

    const auto& args = util::GetArguments(data);
    auto profile = FromJson<std::string>(args, "profileType");

    if (kBluetoothProfileHealth == profile) {
        bool supported = false;
        if (SYSTEM_INFO_ERROR_NONE != system_info_get_platform_bool(kFeatureBluetoothHealth.c_str(),
                                                                    &supported)) {
            LoggerW("Can't check if BT health profile is supported or not");
        }

        if (!supported) {
            throw NotSupportedException("Bluetooth health profile is not supported");
        } else {
            LoggerD("BT health profile is supported");
        }
    } else {
        throw TypeMismatchException("Wrong profile type.");
    }

    util::ReportSuccess(out);
}

void BluetoothAdapter::GetName(const picojson::value& /* data */, picojson::object& out) {
    LoggerD("Entered");

    util::ReportSuccess(picojson::value(get_name()), out);
}

void BluetoothAdapter::GetAddress(const picojson::value& /* data */, picojson::object& out) {
    LoggerD("Entered");

    if (!is_initialized_) {
        throw UnknownException("Bluetooth service is not initialized.");
    }

    std::string str_address = "";
    char* address = nullptr;
    if (BT_ERROR_NONE == bt_adapter_get_address(&address)) {
        if (address) {
            str_address = address;
            free(address);
        }
    }

    util::ReportSuccess(picojson::value(str_address), out);
}

void BluetoothAdapter::GetPowered(const picojson::value& /* data */, picojson::object& out) {
    LoggerD("Entered");

    util::ReportSuccess(picojson::value(is_powered_), out);
}

void BluetoothAdapter::GetVisible(const picojson::value& /* data */, picojson::object& out) {
    LoggerD("Entered");

    util::ReportSuccess(picojson::value(get_visible()), out);
}

void BluetoothAdapter::OnSocketConnected(int result,
                                         bt_socket_connection_state_e state,
                                         bt_socket_connection_s* connection,
                                         void* user_data) {
    LoggerD("Entered");

    BluetoothAdapter* object = static_cast<BluetoothAdapter*>(user_data);

    if (!object) {
        LoggerW("user_data is NULL");
        return;
    }

    if (!connection) {
        LoggerW("connection is NULL");
        return;
    }

    if (BT_SOCKET_SERVER == connection->local_role) {
        LoggerD("Server");

        const auto iter = object->registered_uuids_.find(connection->service_uuid);
        if (iter == object->registered_uuids_.end()) {
            LoggerW("Connection state has changed unexpectedly");
            return;
        }

        if (BT_SOCKET_CONNECTED == state) {  // connected when Server
            if (BT_ERROR_NONE == result) {
                // Update registered_uuids_ state
                iter->second.second = true;

                // Call BluetoothServiceHandler.onconnect
                util::FireEvent("BLUETOOTH_SERVICE_ONCONNECT", BluetoothSocket::ToJson(connection));

                // Update connected_sockets_
                object->connected_sockets_.push_back(connection->socket_fd);
                bt_socket_set_data_received_cb(OnSocketReceivedData, user_data);
            } else {
                LoggerW("Establishing a connection failed");
            }
            return;
        } else {  // disconnected when Server
            if (BT_ERROR_NONE == result) {
                // Update registered_uuids_ state
                iter->second.second = false;

                object->RemoveSocket(connection->socket_fd);
            }
            else {
                LoggerW("Disconnecting a connection failed");
            }
        }
    } else if (BT_SOCKET_CLIENT == connection->local_role) {
        LoggerD("Client");

        if (BT_SOCKET_CONNECTED == state) {  // connected when Client
            const auto range = object->connection_requests_.equal_range(connection->remote_address);
            const auto end = object->connection_requests_.end();
            auto request = end;

            for (auto it = range.first; it != range.second; ++it) {
                if (strcmp(it->second->uuid_.c_str(), connection->service_uuid) == 0) {
                    request = it;
                    break;
                }
            }

            if (end == request) {
                LoggerW("Connection state has changed unexpectedly");
                return;
            }

            std::shared_ptr<picojson::value> response =
                    std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

            if (BT_ERROR_NONE == result) {
                object->connected_sockets_.push_back(connection->socket_fd);
                bt_socket_set_data_received_cb(OnSocketReceivedData, user_data);

                util::ReportSuccess(BluetoothSocket::ToJson(connection),
                                            response->get<picojson::object>());
            } else {
                util::ReportError(NotFoundException("Not found"),
                                          response->get<picojson::object>());
            }

            util::AsyncResponse(request->second->callback_handle_, response);

            // request has been handled, can be safely removed
            object->connection_requests_.erase(request);
        } else {  // disconnected when Client
            if (result == BT_ERROR_NONE) {
                object->RemoveSocket(connection->socket_fd);
            } else {
                LoggerW("Disconnecting a connection failed");
            }
        }
    } else {
        LoggerW("Unknown role");
        return;
    }

    if (object->connected_sockets_.size() == 0) {
        bt_socket_unset_data_received_cb();
    }

    if (object->registered_uuids_.size() == 0 &&
        object->connection_requests_.size() == 0 &&
        object->connected_sockets_.size() == 0) {
        bt_socket_unset_connection_state_changed_cb();
    }
}

void BluetoothAdapter::OnSocketReceivedData(bt_socket_received_data_s* data, void* user_data) {
    LoggerD("Entered");

    BluetoothAdapter* object = static_cast<BluetoothAdapter*>(user_data);

    if (!object) {
        LoggerW("user_data is NULL");
        return;
    }

    if (!data) {
        LoggerW("data is NULL");
        return;
    }

    const auto it = std::find(object->connected_sockets_.begin(),
                              object->connected_sockets_.end(),
                              data->socket_fd);

    if (it == object->connected_sockets_.end()) {
        LoggerW("Unknown connected socket: %d", data->socket_fd);
        return;
    }

    // Store received data
    object->StoreSocketData(data);

    InvokeSocketOnMessageEvent(*it);
}

void BluetoothAdapter::ConnectToServiceByUUID(const std::string& address,
                                              const std::string& uuid,
                                              double callback_handle) {
    LoggerD("Entered");

    if (!IsValidUUID(uuid)) {
        throw InvalidValuesException("Wrong UUID");
    }

    try {
        if (is_powered_) {
            int ret = bt_socket_connect_rfcomm(address.c_str(), uuid.c_str());

            switch (ret) {
                case BT_ERROR_NONE: {
                    LoggerD("bt_socket_connect_rfcomm() succeeded");

                    ConnectionRequestPtr request{new ConnectionRequest()};
                    request->uuid_ = uuid;
                    request->callback_handle_ = callback_handle;
                    connection_requests_.insert(std::make_pair(address, request));

                    bt_socket_set_connection_state_changed_cb(OnSocketConnected, this);
                    break;
                }

                case BT_ERROR_INVALID_PARAMETER:
                case BT_ERROR_REMOTE_DEVICE_NOT_BONDED:
                    throw InvalidValuesException("Invalid value");
                    break;

                default:
                    throw UnknownException("Unknown error");
                    break;
            }
        } else {
            throw ServiceNotAvailableException("Bluetooth device is turned off");
        }
    } catch (const PlatformException& e) {
        std::shared_ptr<picojson::value> response =
                std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
        util::ReportError(e, response->get<picojson::object>());
        util::AsyncResponse(callback_handle, response);
    }
}


static void InvokeSocketEvent(int id, const char* event) {
    picojson::value value = picojson::value(picojson::object());
    picojson::object& value_obj = value.get<picojson::object>();
    value_obj.insert(std::make_pair("id", picojson::value(std::to_string(id))));
    value_obj.insert(std::make_pair("event", picojson::value(event)));
    util::FireEvent("BLUETOOTH_SOCKET_STATE_CHANGED", value);
}

void BluetoothAdapter::InvokeSocketOnMessageEvent(int id) {
    InvokeSocketEvent(id, "onmessage");
}

void BluetoothAdapter::InvokeSocketOnCloseEvent(int id) {
    InvokeSocketEvent(id, "onclose");
}

void BluetoothAdapter::RemoveSocket(int socket) {
    const auto data_it = socket_data_.find(socket);

    if (data_it != socket_data_.end()) {
        socket_data_.erase(data_it);
    } else {
        LoggerD("No stored data for socket: %d", socket);
    }

    const auto it = std::find(connected_sockets_.begin(),
                              connected_sockets_.end(),
                              socket);

    if (it == connected_sockets_.end()) {
        LoggerW("Unknown connected socket: %d", socket);
        return;
    }

    connected_sockets_.erase(it);

    BluetoothAdapter::InvokeSocketOnCloseEvent(*it);
}

void BluetoothAdapter::StoreSocketData(bt_socket_received_data_s* data) {
    LoggerD("Entered");

    auto data_store = socket_data_[data->socket_fd];

    for (int i = 0; i < data->data_size; ++i) {
        data_store.push_back(data->data[i]);
    }
}

const std::list<char>& BluetoothAdapter::ReadSocketData(int socket) {
    LoggerD("Entered");

    return socket_data_[socket];
}

void BluetoothAdapter::ClearSocketData(int socket) {
    LoggerD("Entered");

    const auto data_it = socket_data_.find(socket);

    if (data_it != socket_data_.end()) {
        data_it->second.clear();
    }
}

void BluetoothAdapter::IsServiceConnected(const picojson::value& data, picojson::object& out) {
    LoggerD("Entered");

    const auto& args = util::GetArguments(data);
    const auto& uuid = FromJson<std::string>(args, "uuid");

    auto iter = registered_uuids_.find(uuid);
    if (iter == registered_uuids_.end()) {
        throw InvalidValuesException("Invalid parameter was passed.");
    }

    util::ReportSuccess(picojson::value(iter->second.second), out);
}

} // namespace bluetooth
} // namespace extension
