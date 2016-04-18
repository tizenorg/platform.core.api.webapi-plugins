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
#include "bluetooth/bluetooth_privilege.h"

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

class HexData {
 public:
  HexData()
      : length_(0) {
  }

  void Parse(const std::string& d) {
    const char* p_data = d.c_str();
    int size = d.length();
    if (size > 2 && (d.find("0x", 0) == 0 || d.find("0X", 0) == 0)) {
      p_data += 2;
      size -= 2;
    }
    length_ = size / 2;
    pointer_.reset(new char[length_]);
    common::tools::HexToBin(p_data, size, (unsigned char*)pointer(), length_);
  }

  const char* const pointer() const {
    return pointer_.get();
  }

  const int length() const {
    return length_;
  }

 private:
  std::unique_ptr<char[]> pointer_;
  int length_;
};

class BluetoothLEServiceData : public ParsedDataHolder {
 public:
  const std::string& uuid() const {
    return uuid_;
  }

  const HexData& data() const {
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
      out->data_.Parse(data.get<std::string>());
    } else {
      return false;
    }

    return true;
  }

  std::string uuid_;
  HexData data_;
};

class BluetoothLEManufacturerData : public ParsedDataHolder {
 public:
  const std::string& id() const {
    return id_;
  }

  const HexData& data() const {
    return data_;
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
      out->data_.Parse(val_data.get<std::string>());
      return true;
    } else {
      return false;
    }
  }

  std::string id_;
  HexData data_;
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
  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(Privilege::kBluetooth,
                                              Privilege::kBluetoothAdmin, &out);

  int ret = bt_adapter_le_start_scan(OnScanResult, this);

  if (BT_ERROR_NONE != ret) {
    if (BT_ERROR_NOW_IN_PROGRESS == ret) {
      LogAndReportError(
          PlatformResult(ErrorCode::INVALID_STATE_ERR, "Scan already in progress"), &out,
          ("Scan in progress %d (%s)", ret, get_error_message(ret)));
    } else {

      // other errors are reported asynchronously
      picojson::value value = picojson::value(picojson::object());
      picojson::object* data_obj = &value.get<picojson::object>();
      data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
      LogAndReportError(
          PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to start scan"), data_obj,
          ("Failed to start scan: %d (%s)", ret, get_error_message(ret)));
      instance_.FireEvent(kScanEvent, value);
    }
  } else {
    scanning_ = true;
    ReportSuccess(out);
  }
}

void BluetoothLEAdapter::StopScan(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(Privilege::kBluetooth,
                                              Privilege::kBluetoothAdmin, &out);

  int ret = bt_adapter_le_stop_scan();

  if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
    LogAndReportError(
        PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to stop scan"), &out,
        ("Failed to stop scan: %d (%s)", ret, get_error_message(ret)));
  } else {
    scanning_ = false;
    ReportSuccess(out);
  }
}

void BluetoothLEAdapter::StartAdvertise(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(Privilege::kBluetooth,
                                              Privilege::kBluetoothAdmin, &out);

  const auto& json_advertise_data = data.get("advertiseData");
  const auto& json_packet_type = data.get("packetType");
  const auto& json_mode = data.get("mode");
  const auto& json_connectable = data.get("connectable");

  if (!json_advertise_data.is<picojson::object>() ||
      !json_packet_type.is<std::string>() ||
      !json_mode.is<std::string>() ||
      !json_connectable.is<bool>()) {
    LogAndReportError(
        PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected parameter type"), &out);
    return;
  }

  BluetoothLEAdvertiseData advertise_data;
  if (!BluetoothLEAdvertiseData::Construct(json_advertise_data, &advertise_data)) {
    LogAndReportError(
        PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of advertise data"), &out);
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
      LogAndReportError(
          PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of packet type"), &out,
          ("Wrong packet_type: %s", str_packet_type.c_str()));
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
      LogAndReportError(
          PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Unexpected value of mode"), &out,
          ("Wrong mode: %s", str_mode.c_str()));
      return;
    }
  }

  if (nullptr != bt_advertiser_) {
    LogAndReportError(
        PlatformResult(ErrorCode::INVALID_STATE_ERR, "Advertise already in progress"), &out);
    return;
  }

  bt_advertiser_h advertiser = nullptr;

  int ret = bt_adapter_le_create_advertiser(&advertiser);
  if (BT_ERROR_NONE != ret) {
    LogAndReportError(
        util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
        ("bt_adapter_le_create_advertiser() failed with: %d, (%s)", ret, get_error_message(ret)));
    return;
  }

  std::unique_ptr<std::remove_pointer<bt_advertiser_h>::type,
      int (*)(bt_advertiser_h)> advertiser_ptr(advertiser, &bt_adapter_le_destroy_advertiser);  // automatically release the memory

  // configure advertiser
  if (advertise_data.include_name()) {
    ret = bt_adapter_le_set_advertising_device_name(advertiser, packet_type,
                                                advertise_data.include_name());
    if (BT_ERROR_NONE != ret) {
      LogAndReportError(
          util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
          ("bt_adapter_le_set_advertising_device_name() failed with: %d (%s)", ret, get_error_message(ret)));
      return;
    }
  }

  for (const auto& i : advertise_data.service_uuids()) {
    ret = bt_adapter_le_add_advertising_service_uuid(advertiser, packet_type,
                                                     i.c_str());
    if (BT_ERROR_NONE != ret) {
      LogAndReportError(
          util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
          ("bt_adapter_le_add_advertising_service_uuid() failed with: %d (%s)", ret, get_error_message(ret)));
      return;
    }
  }

  for (const auto& i : advertise_data.solicitation_uuids()) {
    ret = bt_adapter_le_add_advertising_service_solicitation_uuid(advertiser,
                                                                  packet_type,
                                                                  i.c_str());
    if (BT_ERROR_NONE != ret) {
      LogAndReportError(
          util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
          ("bt_adapter_le_add_advertising_service_solicitation_uuid() failed with: %d (%s)",
            ret, get_error_message(ret)));
      return;
    }
  }

  ret = bt_adapter_le_set_advertising_appearance(advertiser, packet_type,
                                                 advertise_data.appearance());
  if (BT_ERROR_NONE != ret) {
    LogAndReportError(
        util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
        ("bt_adapter_le_set_advertising_appearance() failed with: %d (%s)",
          ret, get_error_message(ret)));
    return;
  }

  if (advertise_data.include_tx_power_level()) {
    ret = bt_adapter_le_set_advertising_tx_power_level(advertiser, packet_type,
                                                       advertise_data.include_tx_power_level());
    if (BT_ERROR_NONE != ret) {
      LogAndReportError(
          util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
          ("bt_adapter_le_set_advertising_tx_power_level() failed with: %d (%s)",
            ret, get_error_message(ret)));
      return;
    }
  }

  const auto& service_data = advertise_data.service_data();
  if (service_data.uuid().empty() && nullptr == service_data.data().pointer()) {
    LoggerD("service data is empty");
  } else {
    ret = bt_adapter_le_add_advertising_service_data(advertiser, packet_type,
                                                     service_data.uuid().c_str(),
                                                     service_data.data().pointer(),
                                                     service_data.data().length());
    if (BT_ERROR_NONE != ret) {
      LogAndReportError(
          util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
          ("bt_adapter_le_add_advertising_service_data() failed with: %d (%s)",
            ret, get_error_message(ret)));
      return;
    }
  }

  const auto& manufacturer_data = advertise_data.manufacturer_data();
  if (manufacturer_data.id().empty() && nullptr == manufacturer_data.data().pointer()) {
    LoggerD("manufacturerData is empty");
  } else {
    if (manufacturer_data.valid()) {
      ret = bt_adapter_le_add_advertising_manufacturer_data(advertiser,
                                                            packet_type,
                                                            atoi(manufacturer_data.id().c_str()),
                                                            manufacturer_data.data().pointer(),
                                                            manufacturer_data.data().length());
      if (BT_ERROR_NONE != ret) {
        LogAndReportError(
            util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
            ("bt_adapter_le_add_advertising_manufacturer_data() failed with: %d (%s)",
              ret, get_error_message(ret)));
        return;
      }
    }
  }

  ret = bt_adapter_le_set_advertising_mode(advertiser, mode);
  if (BT_ERROR_NONE != ret) {
    LogAndReportError(
        util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
        ("bt_adapter_le_set_advertising_mode() failed with: %d (%s)",
          ret, get_error_message(ret)));
    return;
  }

  ret = bt_adapter_le_set_advertising_connectable(advertiser, json_connectable.get<bool>());
  if (BT_ERROR_NONE != ret) {
    LogAndReportError(
        util::GetBluetoothError(ret, "Failed to create advertiser"), &out,
        ("bt_adapter_le_set_advertising_connectable() failed with: %d (%s)",
          ret, get_error_message(ret)));
    return;
  }

  // advertiser is ready, let's start advertising
  ret = bt_adapter_le_start_advertising_new(advertiser, OnAdvertiseResult, this);
  if (BT_ERROR_NONE != ret) {
    if (BT_ERROR_NOW_IN_PROGRESS == ret) {
      LogAndReportError(
          PlatformResult(ErrorCode::INVALID_STATE_ERR, "Advertise already in progress"), &out,
          ("bt_adapter_le_start_advertising_new error: %d (%s)", ret, get_error_message(ret)));
      return;
    }

    LogAndReportError(
        util::GetBluetoothError(ret, "Failed to start advertising"), &out,
        ("bt_adapter_le_start_advertising_new() failed with: %d (%s)",
          ret, get_error_message(ret)));
    return;
  }

  // everything went well, we want to store the pointer, so unique_ptr should no longer manage the memory
  bt_advertiser_ = advertiser_ptr.release();
  ReportSuccess(out);
}

void BluetoothLEAdapter::StopAdvertise(const picojson::value& data, picojson::object& out) {
  LoggerD("Entered");
  CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(Privilege::kBluetooth,
                                              Privilege::kBluetoothAdmin, &out);

  if (nullptr != bt_advertiser_) {
    int ret = bt_adapter_le_stop_advertising(bt_advertiser_);
    if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
      LogAndReportError(
          PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to stop advertising"), &out,
          ("bt_adapter_le_stop_advertising() failed with: %d (%s)",
            ret, get_error_message(ret)));
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
    LogAndReportError(
        util::GetBluetoothError(result, "Error during scanning"), data_obj,
        ("Error during scanning: %d (%s)", result, get_error_message(result)));
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanError)));
  } else {
    // this is probably capi-network-bluetooth error: when scan is stopped info has 0x1 value
    if (nullptr != info && reinterpret_cast<void*>(0x1) != info) {
      // device found
      LoggerD("Device found");
      picojson::value data{picojson::object{}};
      const auto& r = BluetoothLEDevice::ToJson(info, &data.get<picojson::object>());
      if (r) {
        data_obj->insert(std::make_pair(kAction, picojson::value(kOnScanSuccess)));
        data_obj->insert(std::make_pair(kData, data));
      } else {
        LogAndReportError(r, data_obj, ("Failed to parse Bluetooth LE device"));
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
    LogAndReportError(
        util::GetBluetoothError(result, "Error during advertising"), data_obj,
        ("Error during advertising: %d (%s)", result, get_error_message(result)));
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnAdvertiseError)));
  } else {
    const char* state = (BT_ADAPTER_LE_ADVERTISING_STARTED == adv_state) ? "STARTED" : "STOPPED";
    LoggerD("Advertise state is: %s", state);
    data_obj->insert(std::make_pair(kAction, picojson::value(kOnAdvertiseState)));
    ReportSuccess(picojson::value(state), *data_obj);
    if (adv_state == BT_ADAPTER_LE_ADVERTISING_STOPPED){
      LoggerD("Advertiser destroy");
      int  ret = bt_adapter_le_destroy_advertiser(advertiser);
      if (BT_ERROR_NONE != ret && BT_ERROR_NOT_IN_PROGRESS != ret) {
        LogAndReportError(
            PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to destroy advertiser"), data_obj,
            ("bt_adapter_le_destroy_advertiser() failed with: %d (%s)",
              ret, get_error_message(ret)));
        return;
      }
    }
  }

  adapter->instance_.FireEvent(kAdvertiseEvent, value);
}

} // namespace bluetooth
} // namespace extension
