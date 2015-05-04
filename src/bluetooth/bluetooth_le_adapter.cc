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

#include "bluetooth/bluetooth_le_adapter.h"

#include "common/logger.h"

#include "bluetooth/bluetooth_instance.h"
#include "bluetooth/bluetooth_util.h"

namespace extension {
namespace bluetooth {

namespace {

// TODO: remove this code when BluetoothLEDevice is available
class BluetoothLEDevice {
 public:
  static common::PlatformResult ToJson(
       bt_adapter_le_device_scan_result_info_s* info,
       picojson::object* le_device) {
    return common::PlatformResult(common::ErrorCode::NO_ERROR);
  }
};
// TODO end

// utility functions

bool ToBool(bt_adapter_le_state_e state) {
  return (BT_ADAPTER_LE_ENABLED == state) ? true : false;
}

// constants

const std::string kAction = "action";
const std::string kData = "data";
// scan-related
const std::string kOnScanStarted = "onstarted";
const std::string kOnScanDeviceFound = "ondevicefound";
const std::string kOnScanFinished = "onfinished";
const std::string kOnScanError = "onerror";
const std::string kScanEvent = "BluetoothLEScanCallback";

} // namespace

using common::ErrorCode;
using common::PlatformResult;
using common::tools::ReportError;
using common::tools::ReportSuccess;

BluetoothLEAdapter::BluetoothLEAdapter(BluetoothInstance& instance)
    : instance_(instance),
      enabled_(false),
      scanning_(false) {
  LoggerD("Entered");

  bt_adapter_le_state_e le_state = BT_ADAPTER_LE_DISABLED;

  int ret = bt_adapter_le_get_state(&le_state);

  if (BT_ERROR_NONE == ret) {
    enabled_ = ToBool(le_state);

    ret = bt_adapter_le_set_state_changed_cb(OnStateChanged, this);

    if (BT_ERROR_NONE == ret) {
      if (!enabled_) {
        LoggerD("BTLE is not enabled, turning on...");
        // enabled_ is going to be updated by OnStateChanged callback
        ret = bt_adapter_le_enable();

        if (BT_ERROR_NONE != ret) {
          LoggerE("Failed to enable BTLE.");
        }
      }
    } else {
      LoggerE("Failed to register BTLE state changed listener.");
    }
  } else {
    LoggerE("Failed to obtain current state of BTLE.");
  }
}

BluetoothLEAdapter::~BluetoothLEAdapter() {
  bt_adapter_le_unset_state_changed_cb();
  if (scanning_) {
    bt_adapter_le_stop_scan();
  }
  bt_adapter_le_disable();
}

void BluetoothLEAdapter::StartScan(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  int ret = bt_adapter_le_start_scan(OnScanResult, this);

  if (BT_ERROR_NONE != ret) {
    if (BT_ERROR_NOW_IN_PROGRESS == ret) {
      LoggerE("Scan in progress");
      ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR, "Scan already in progress"), &out);
    } else {
      LoggerE("Failed to start scan: %d", ret);

      // other errors are reported asynchronously
      picojson::value value = picojson::value(picojson::object());
      picojson::object* data_obj = &value.get<picojson::object>();
      data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to start scan"), data_obj);
      instance_.FireEvent(kScanEvent, value);
    }
  } else {
    scanning_ = true;
    ReportSuccess(out);
  }
}

void BluetoothLEAdapter::StopScan(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  int ret = bt_adapter_le_stop_scan();

  if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
    LoggerE("Failed to stop scan: %d", ret);
    ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to stop scan"), &out);
  } else {
    scanning_ = false;
    ReportSuccess(out);
  }
}

void BluetoothLEAdapter::StartAdvertise(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEAdapter::StopAdvertise(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
}

void BluetoothLEAdapter::OnStateChanged(int result,
                                        bt_adapter_le_state_e adapter_le_state,
                                        void* user_data) {
  LoggerD("Entered");

  auto adapter = static_cast<BluetoothLEAdapter*>(user_data);

  if (!adapter) {
    LoggerE("user_data is NULL");
    return;
  }

  adapter->enabled_ = ToBool(adapter_le_state);
}

void BluetoothLEAdapter::OnScanResult(
    int result, bt_adapter_le_device_scan_result_info_s* info,
    void* user_data) {
  LoggerD("Entered, result: %d, info: %p, data: %p", result, info, user_data);

  auto adapter = static_cast<BluetoothLEAdapter*>(user_data);

  if (!adapter) {
    LoggerE("user_data is NULL");
    return;
  }

  picojson::value value = picojson::value(picojson::object());
  picojson::object* data_obj = &value.get<picojson::object>();

  if (BT_ERROR_NONE != result) {
    LoggerE("Error during scanning: %d", result);
    ReportError(util::GetBluetoothError(result, "Error during scanning"), data_obj);
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
  } else {
    // TODO: this is probably capi-network-bluetooth error: when scan is stopped info has 0x1 value
    if (nullptr == info || reinterpret_cast<void*>(0x1) == info) {
      // info is empty, so this is start/stop callback
      if (!adapter->scanning_) { // scanning has to be stopped by the user, it is not stopped by the platform
        LoggerD("Scan finished");
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanFinished)));
        data_obj->insert(std::make_pair(kData, picojson::value(adapter->discovered_devices_)));
      } else {
        LoggerD("Scan started");
        adapter->discovered_devices_.clear();
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanStarted)));
      }
    } else {
      // device found
      LoggerD("Device found");
      picojson::value data{picojson::object{}};
      const auto& r = BluetoothLEDevice::ToJson(info, &data.get<picojson::object>());
      if (r) {
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanDeviceFound)));
        data_obj->insert(std::make_pair(kData, data));
        adapter->discovered_devices_.push_back(data);
      } else {
        LoggerE("Failed to parse Bluetooth LE device");
        ReportError(r, data_obj);
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
      }
    }
  }

  adapter->instance_.FireEvent(kScanEvent, value);
}

} // namespace bluetooth
} // namespace extension
