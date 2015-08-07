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

#include "bluetooth/bluetooth_le_adapter.h"

#include "common/tools.h"
#include "common/logger.h"

#include "bluetooth/bluetooth_instance.h"
#include "bluetooth/bluetooth_le_device.h"
#include "bluetooth/bluetooth_util.h"

namespace extension {
namespace bluetooth {

namespace {

class ParsedDataHolder {
 public:
  ParsedDataHolder() : valid_(false) {}
  virtual ~ParsedDataHolder() {}

  bool valid() const {
    return valid_;
  }

 protected:
  void set_valid() {
    valid_ = true;
  }

 private:
  bool valid_;
};

class BluetoothLEServiceData : public ParsedDataHolder {
 public:
  BluetoothLEServiceData()
      : ParsedDataHolder() {
  }

  const std::string& uuid() const {
    return uuid_;
  }

  const std::string& data() const {
    return data_;
  }

  static bool Construct(const picojson::value& obj,
                        BluetoothLEServiceData* out) {
    if (!obj.is<picojson::object>() ||
        !ParseUUID(obj, out) ||
        !ParseData(obj, out)) {
      return false;
    }

    out->set_valid();

    return true;
  }

 private:
  static bool ParseUUID(const picojson::value& obj,
                        BluetoothLEServiceData* out) {
    LoggerD("Entered");
    const auto& uuid = obj.get("serviceuuid");
    if (uuid.is<std::string>()) {
      out->uuid_ = uuid.get<std::string>();
    } else {
      return false;
    }

    return true;
  }

  static bool ParseData(const picojson::value& obj,
                        BluetoothLEServiceData* out) {
    LoggerD("Entered");
    const auto& data = obj.get("data");
    if (data.is<std::string>()) {
      out->data_ = data.get<std::string>();
    } else {
      return false;
    }

    return true;
  }

  std::string uuid_;
  std::string data_;
};

class BluetoothLEManufacturerData : public ParsedDataHolder {
 public:
  BluetoothLEManufacturerData()
      : ParsedDataHolder(),
        data_(nullptr),
        data_length_(0) {
  }

  const std::string& id() const {
    return id_;
  }

  const unsigned char* const data() const {
    return data_;
  }

  const int data_length() const {
    return data_length_;
  }

  static bool Construct(const picojson::value& obj,
                        BluetoothLEManufacturerData* out) {
    LoggerD("Entered");
    if (!obj.is<picojson::object>() ||
        !ParseId(obj, out) ||
        !ParseData(obj, out)) {
      return false;
    }

    out->set_valid();

    return true;
  }

  ~BluetoothLEManufacturerData() {
    if (data_) {
      delete [] data_;
      data_ = nullptr;
      data_length_ = 0;
    }
  }

 private:
  static bool ParseId(const picojson::value& obj,
                      BluetoothLEManufacturerData* out) {
    LoggerD("Entered");
    const auto& id = obj.get("id");
    if (id.is<std::string>()) {
      out->id_ = id.get<std::string>();
    } else {
      return false;
    }

    return true;
  }

  static bool ParseData(const picojson::value& obj,
                        BluetoothLEManufacturerData* out) {
    LoggerD("Entered");

    const auto& val_data = obj.get("data");

    if (val_data.is<std::string>()) {
      const std::string& str_data = val_data.get<std::string>();
      const char* p_data = str_data.c_str();
      int size = str_data.length();
      if (size > 2 && (str_data.find("0x", 0) == 0 || str_data.find("0X", 0) == 0)) {
        p_data += 2;
        size -= 2;
      }
      out->data_length_ = size / 2;
      out->data_ = new unsigned char[out->data_length_];
      common::tools::HexToBin(p_data, size, out->data_, out->data_length_);
      return true;
    } else {
      return false;
    }
  }

  std::string id_;
  unsigned char* data_;
  int data_length_;
};

class BluetoothLEAdvertiseData : public ParsedDataHolder {
 public:
  BluetoothLEAdvertiseData()
      : ParsedDataHolder(),
        include_name_(false),
        appearance_(0),  // 0 means unknown
        include_tx_power_level_(false) {
  }

  bool include_name() const {
    return include_name_;
  }

  const std::vector<std::string>& service_uuids() const {
    return service_uuids_;
  }

  const std::vector<std::string>& solicitation_uuids() const {
    return solicitation_uuids_;
  }

  int appearance() const {
    return appearance_;
  }

  bool include_tx_power_level() const {
    return include_tx_power_level_;
  }

  const BluetoothLEServiceData& service_data() const {
    return service_data_;
  }

  const BluetoothLEManufacturerData& manufacturer_data() const {
    return manufacturer_data_;
  }

  static bool Construct(const picojson::value& obj,
                        BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    if (!obj.is<picojson::object>() ||
        !ParseIncludeName(obj, out) ||
        !ParseServiceUUIDs(obj, out) ||
        !ParseSolicitationUUIDs(obj, out) ||
        !ParseAppearance(obj, out) ||
        !ParseIncludeTxPowerLevel(obj, out) ||
        !ParseServiceData(obj, out) ||
        !ParseManufacturerData(obj, out)) {
      return false;
    }

    out->set_valid();

    return true;
  }

 private:
  static bool ParseIncludeName(const picojson::value& obj,
                               BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& include_name = obj.get("includeName");
    if (include_name.is<bool>()) {
      out->include_name_ = include_name.get<bool>();
    } else if (!include_name.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseServiceUUIDs(const picojson::value& obj,
                                BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& service_uuids = obj.get("serviceuuids");
    if (service_uuids.is<picojson::array>()) {
      for (const auto& i : service_uuids.get<picojson::array>()) {
        if (i.is<std::string>()) {
          out->service_uuids_.push_back(i.get<std::string>());
        } else {
          return false;
        }
      }
    } else if (!service_uuids.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseSolicitationUUIDs(const picojson::value& obj,
                                     BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& solicitation_uuids = obj.get("solicitationuuids");
    if (solicitation_uuids.is<picojson::array>()) {
      for (const auto& i : solicitation_uuids.get<picojson::array>()) {
        if (i.is<std::string>()) {
          out->solicitation_uuids_.push_back(i.get<std::string>());
        } else {
          return false;
        }
      }
    } else if (!solicitation_uuids.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseAppearance(const picojson::value& obj,
                              BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& appearance = obj.get("appearance");
    if (appearance.is<double>()) {
      out->appearance_ = static_cast<decltype(appearance_)>(appearance.get<double>());
    } else if (!appearance.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseIncludeTxPowerLevel(const picojson::value& obj,
                                       BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& include_tx_power_level = obj.get("includeTxPowerLevel");
    if (include_tx_power_level.is<bool>()) {
      out->include_tx_power_level_ = include_tx_power_level.get<bool>();
    } else if (!include_tx_power_level.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseServiceData(const picojson::value& obj,
                               BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& service_data = obj.get("serviceData");
    BluetoothLEServiceData data;
    if (BluetoothLEServiceData::Construct(service_data, &data)) {
      out->service_data_ = std::move(data);
    } else if (!service_data.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  static bool ParseManufacturerData(const picojson::value& obj,
                                    BluetoothLEAdvertiseData* out) {
    LoggerD("Entered");
    const auto& manufacturer_data = obj.get("manufacturerData");
    BluetoothLEManufacturerData data;
    if (BluetoothLEManufacturerData::Construct(manufacturer_data, &data)) {
      out->manufacturer_data_ = std::move(data);
    } else if (!manufacturer_data.is<picojson::null>()) {
      return false;
    }

    return true;
  }

  bool include_name_;
  std::vector<std::string> service_uuids_;
  std::vector<std::string> solicitation_uuids_;
  int appearance_;
  bool include_tx_power_level_;
  BluetoothLEServiceData service_data_;
  BluetoothLEManufacturerData manufacturer_data_;
};

// utility functions

bool ToBool(bt_adapter_le_state_e state) {
  return (BT_ADAPTER_LE_ENABLED == state) ? true : false;
}

// constants

const std::string kAction = "action";
const std::string kData = "data";
// scan-related
const std::string kOnScanSuccess = "onsuccess";
const std::string kOnScanError = "onerror";
const std::string kScanEvent = "BluetoothLEScanCallback";
// advertise-related
const std::string kOnAdvertiseState = "onstate";
const std::string kOnAdvertiseError = "onerror";
const std::string kAdvertiseEvent = "BluetoothLEAdvertiseCallback";

} // namespace

using common::ErrorCode;
using common::PlatformResult;
using common::tools::ReportError;
using common::tools::ReportSuccess;

BluetoothLEAdapter::BluetoothLEAdapter(BluetoothInstance& instance)
    : instance_(instance),
      enabled_(false),
      scanning_(false),
      bt_advertiser_(nullptr) {
  LoggerD("Entered");

  bt_adapter_le_state_e le_state = BT_ADAPTER_LE_DISABLED;

  int ret = bt_adapter_le_get_state(&le_state);

  if (BT_ERROR_NONE == ret) {
    enabled_ = ToBool(le_state);

    ret = bt_adapter_le_set_state_changed_cb(OnStateChanged, this);
    if (BT_ERROR_NONE != ret) {
      LoggerE("Failed to register BTLE state changed listener.");
    }
  } else {
    LoggerE("Failed to obtain current state of BTLE.");
  }
}

BluetoothLEAdapter::~BluetoothLEAdapter() {
  LoggerD("Entered");
  bt_adapter_le_unset_state_changed_cb();
  if (scanning_) {
    bt_adapter_le_stop_scan();
  }
  if (bt_advertiser_) {
    bt_adapter_le_stop_advertising(bt_advertiser_);
    bt_adapter_le_destroy_advertiser(bt_advertiser_);
  }
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

  const auto& json_advertise_data = data.get("advertiseData");
  const auto& json_packet_type = data.get("packetType");
  const auto& json_mode = data.get("mode");
  const auto& json_connectable = data.get("connectable");

  if (!json_advertise_data.is<picojson::object>() ||
      !json_packet_type.is<std::string>() ||
      !json_mode.is<std::string>() ||
      !json_connectable.is<bool>()) {
    ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected parameter type"), &out);
    return;
  }

  BluetoothLEAdvertiseData advertise_data;
  if (!BluetoothLEAdvertiseData::Construct(json_advertise_data, &advertise_data)) {
    ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of advertise data"), &out);
    return;
  }

  bt_adapter_le_packet_type_e packet_type = BT_ADAPTER_LE_PACKET_ADVERTISING;
  {
    const auto& str_packet_type = json_packet_type.get<std::string>();
    if ("ADVERTISE" == str_packet_type) {
      packet_type = BT_ADAPTER_LE_PACKET_ADVERTISING;
    } else if ("SCAN_RESPONSE" == str_packet_type) {
      packet_type = BT_ADAPTER_LE_PACKET_SCAN_RESPONSE;
    } else {
      LoggerE("Fail: json_packet_type.get");
      ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of packet type"), &out);
      return;
    }
  }

  bt_adapter_le_advertising_mode_e mode = BT_ADAPTER_LE_ADVERTISING_MODE_BALANCED;
  {
    const auto& str_mode = json_mode.get<std::string>();
    if ("BALANCED" == str_mode) {
      mode = BT_ADAPTER_LE_ADVERTISING_MODE_BALANCED;
    } else if ("LOW_LATENCY" == str_mode) {
      mode = BT_ADAPTER_LE_ADVERTISING_MODE_LOW_LATENCY;
    } else if ("LOW_ENERGY" == str_mode) {
      mode = BT_ADAPTER_LE_ADVERTISING_MODE_LOW_ENERGY;
    } else {
      ReportError(PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of mode"), &out);
      return;
    }
  }

  if (nullptr != bt_advertiser_) {
    LoggerE("Advertise in progress");
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR, "Advertise already in progress"), &out);
    return;
  }

  bt_advertiser_h advertiser = nullptr;

  int ret = bt_adapter_le_create_advertiser(&advertiser);
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_adapter_le_create_advertiser() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
    return;
  }

  std::unique_ptr<std::remove_pointer<bt_advertiser_h>::type,
      int (*)(bt_advertiser_h)> advertiser_ptr(advertiser, &bt_adapter_le_destroy_advertiser);  // automatically release the memory

  // configure advertiser
  if (advertise_data.include_name()) {
    ret = bt_adapter_le_set_advertising_device_name(advertiser, packet_type,
                                                advertise_data.include_name());
    if (BT_ERROR_NONE != ret) {
      LoggerE("bt_adapter_le_set_advertising_device_name() failed with: %d", ret);
      ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
      return;
    }
  }

  for (const auto& i : advertise_data.service_uuids()) {
    ret = bt_adapter_le_add_advertising_service_uuid(advertiser, packet_type,
                                                     i.c_str());
    if (BT_ERROR_NONE != ret) {
      LoggerE("bt_adapter_le_add_advertising_service_uuid() failed with: %d", ret);
      ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
      return;
    }
  }

  for (const auto& i : advertise_data.solicitation_uuids()) {
    ret = bt_adapter_le_add_advertising_service_solicitation_uuid(advertiser,
                                                                  packet_type,
                                                                  i.c_str());
    if (BT_ERROR_NONE != ret) {
      LoggerE("bt_adapter_le_add_advertising_service_solicitation_uuid() failed with: %d", ret);
      ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
      return;
    }
  }

  ret = bt_adapter_le_set_advertising_appearance(advertiser, packet_type,
                                                 advertise_data.appearance());
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_adapter_le_set_advertising_appearance() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
    return;
  }

  if (advertise_data.include_tx_power_level()) {
    ret = bt_adapter_le_set_advertising_tx_power_level(advertiser, packet_type,
                                                       advertise_data.include_tx_power_level());
    if (BT_ERROR_NONE != ret) {
      LoggerE("bt_adapter_le_set_advertising_tx_power_level() failed with: %d", ret);
      ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
      return;
    }
  }

  const auto& service_data = advertise_data.service_data();
  if (service_data.uuid().empty() && service_data.data().empty()) {
    LoggerD("service data is empty");
  } else {
    ret = bt_adapter_le_add_advertising_service_data(advertiser, packet_type,
                                                     service_data.uuid().c_str(),
                                                     service_data.data().c_str(),
                                                     service_data.data().length());
    if (BT_ERROR_NONE != ret) {
      LoggerE("bt_adapter_le_add_advertising_service_data() failed with: %d", ret);
      ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
      return;
    }
  }

  const auto& manufacturer_data = advertise_data.manufacturer_data();
  if (manufacturer_data.id().empty() && manufacturer_data.data() == nullptr) {
    LoggerD("manufacturerData is empty");
  } else {
    if (manufacturer_data.valid()) {
      ret = bt_adapter_le_add_advertising_manufacturer_data(advertiser,
                                                            packet_type,
                                                            atoi(manufacturer_data.id().c_str()),
                                                            (const char*)manufacturer_data.data(),
                                                            manufacturer_data.data_length());
      if (BT_ERROR_NONE != ret) {
        LoggerE("bt_adapter_le_add_advertising_manufacturer_data() failed with: %d", ret);
        ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
        return;
      }
    }
  }

  ret = bt_adapter_le_set_advertising_mode(advertiser, mode);
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_adapter_le_set_advertising_mode() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
    return;
  }

  ret = bt_adapter_le_set_advertising_connectable(advertiser, json_connectable.get<bool>());
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_adapter_le_set_advertising_connectable() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to create advertiser"), &out);
    return;
  }

  // advertiser is ready, let's start advertising
  ret = bt_adapter_le_start_advertising_new(advertiser, OnAdvertiseResult, this);
  if (BT_ERROR_NONE != ret) {
    if (BT_ERROR_NOW_IN_PROGRESS == ret) {
      ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR, "Advertise already in progress"), &out);
      return;
    }

    LoggerE("bt_adapter_le_start_advertising_new() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to start advertising"), &out);
    return;
  }

  // everything went well, we want to store the pointer, so unique_ptr should no longer manage the memory
  bt_advertiser_ = advertiser_ptr.release();
  ReportSuccess(out);
}

void BluetoothLEAdapter::StopAdvertise(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");

  if (nullptr != bt_advertiser_) {
    int ret = bt_adapter_le_stop_advertising(bt_advertiser_);
    if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
      LoggerE("bt_adapter_le_stop_advertising() failed with: %d", ret);
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to stop advertising"), &out);
      return;
    }

    ret = bt_adapter_le_destroy_advertiser(bt_advertiser_);
    if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
      LoggerE("bt_adapter_le_destroy_advertiser() failed with: %d", ret);
      ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to destroy advertiser"), &out);
      return;
    }

    bt_advertiser_ = nullptr;
  } else {
    LoggerD("Advertising is not in progress");
  }

  ReportSuccess(out);
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
    if (nullptr != info && reinterpret_cast<void*>(0x1) != info) {
      // device found
      LoggerD("Device found");
      picojson::value data{picojson::object{}};
      const auto& r = BluetoothLEDevice::ToJson(info, &data.get<picojson::object>());
      if (r) {
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanSuccess)));
        data_obj->insert(std::make_pair(kData, data));
      } else {
        LoggerE("Failed to parse Bluetooth LE device");
        ReportError(r, data_obj);
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
      }
    }
  }

  adapter->instance_.FireEvent(kScanEvent, value);
}

void BluetoothLEAdapter::OnAdvertiseResult(
    int result, bt_advertiser_h advertiser,
    bt_adapter_le_advertising_state_e adv_state, void* user_data) {
  LoggerD("Entered, result: %d, advertiser: %p, adv_state: %d, user_data: %p", result, advertiser, adv_state, user_data);

  auto adapter = static_cast<BluetoothLEAdapter*>(user_data);

  if (!adapter) {
    LoggerE("user_data is NULL");
    return;
  }

  picojson::value value = picojson::value(picojson::object());
  picojson::object* data_obj = &value.get<picojson::object>();

  if (BT_ERROR_NONE != result) {
    LoggerE("Error during advertising: %d", result);
    ReportError(util::GetBluetoothError(result, "Error during advertising"), data_obj);
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnAdvertiseError)));
  } else {
    const char* state = (BT_ADAPTER_LE_ADVERTISING_STARTED == adv_state) ? "STARTED" : "STOPPED";
    LoggerD("Advertise state is: %s", state);
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnAdvertiseState)));
    ReportSuccess(picojson::value(state), *data_obj);
  }

  adapter->instance_.FireEvent(kAdvertiseEvent, value);
}

} // namespace bluetooth
} // namespace extension
