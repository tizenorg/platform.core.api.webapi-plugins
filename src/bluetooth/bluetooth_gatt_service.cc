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

#include "bluetooth/bluetooth_gatt_service.h"

#include <sstream>

#include "common/logger.h"
#include "common/platform_result.h"
#include "common/extension.h"
#include "common/task-queue.h"

#include "bluetooth/bluetooth_instance.h"
#include "bluetooth/bluetooth_util.h"

namespace extension {
namespace bluetooth {

using common::PlatformResult;
using common::ErrorCode;
using common::TaskQueue;
using namespace common::tools;

namespace {
const std::string kUuid = "uuid";
const std::string kHandle = "handle";
const std::string kAddress = "address";

const std::string kDescriptors = "descriptors";
const std::string kBroadcast = "isBroadcast";
const std::string kExtendedProperties = "hasExtendedProperties";
const std::string kNotify = "isNotify";
const std::string kIndication = "isIndication";
const std::string kReadable = "isReadable";
const std::string kSignedWrite = "isSignedWrite";
const std::string kWritable = "isWritable";
const std::string kWriteNoResponse = "isWriteNoResponse";

const std::string kOnValueChanged = "BluetoothGATTCharacteristicValueChangeListener";

bool IsProperty (int propertyBits, bt_gatt_property_e property) {
  return (propertyBits & property) != 0;
}
}

BluetoothGATTService::BluetoothGATTService(BluetoothInstance& instance) :
        instance_(instance)
{
  LoggerD("Entered");
}

BluetoothGATTService::~BluetoothGATTService() {
  LoggerD("Entered");

  for (auto it : gatt_characteristic_) {
    // unregister callback, ignore errors
    bt_gatt_client_unset_characteristic_value_changed_cb(it);
  }

  for (auto it : gatt_clients_) {
    LoggerD("destroying client for address: %s", it.first.c_str());
    bt_gatt_client_destroy(it.second);
  }
}

bool BluetoothGATTService::IsStillConnected(const std::string& address) {
  auto it = gatt_clients_.find(address);
  return gatt_clients_.end() != it;
}

bt_gatt_client_h BluetoothGATTService::GetGattClient(const std::string& address) {
  LoggerD("Entered");

  bt_gatt_client_h client = nullptr;

  const auto it = gatt_clients_.find(address);

  if (gatt_clients_.end() == it) {
    int ret = bt_gatt_client_create(address.c_str(), &client);
    if (BT_ERROR_NONE != ret) {
      LoggerE("Failed to create GATT client, error: %d", ret);
    } else {
      gatt_clients_.insert(std::make_pair(address, client));
    }
  } else {
    LoggerD("Client already created");
    client = it->second;
  }

  return client;
}

// this method should be used to inform this object that some device was disconnected
void BluetoothGATTService::TryDestroyClient(const std::string &address) {
  auto it = gatt_clients_.find(address);
  if (gatt_clients_.end() != it) {
    LoggerD("destroying client for address: %s", it->first.c_str());
    bt_gatt_client_destroy(it->second);
    gatt_clients_.erase(it);
  } else {
    LoggerD("Client for address: %s does not exist, no need for deletion",
            address.c_str());
  }
}

PlatformResult BluetoothGATTService::GetSpecifiedGATTService(const std::string &address,
                                                             const std::string &uuid,
                                                             picojson::object* result) {
  LoggerD("Entered");

  bt_gatt_client_h client = GetGattClient(address);

  if (nullptr == client) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to create the GATT client's handle");
  }

  bt_gatt_h service = nullptr;
  int ret = bt_gatt_client_get_service(client, uuid.c_str(), &service);
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_gatt_client_get_service() error: %d", ret);
    switch (ret) {
      case BT_ERROR_NO_DATA:
        return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Service not found");

      case BT_ERROR_INVALID_PARAMETER:
        return PlatformResult(ErrorCode::NOT_FOUND_ERR, "Service UUID is invalid");

      default:
        return PlatformResult(ErrorCode::UNKNOWN_ERR, "Failed to get a service's GATT handle");
    }
  }

  //report BluetoothGattService
  result->insert(std::make_pair(kUuid, picojson::value(uuid)));
  //handle is passed to upper layer because there is no need to delete it
  result->insert(std::make_pair(kHandle, picojson::value((double)(long)service)));
  //address is necessary to later check if device is still connected
  result->insert(std::make_pair(kAddress, picojson::value(address)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::GetServices(const picojson::value& args,
                                       picojson::object& out) {
  LoggerD("Entered");

  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());
  const std::string& address = args.get("address").get<std::string>();

  picojson::array array;
  PlatformResult ret = GetServicesHelper(handle, address, &array);
  if (ret.IsError()) {
    LoggerE("Error while getting services");
    ReportError(ret, &out);
  } else {
    ReportSuccess(picojson::value(array), out);
  }
}

PlatformResult BluetoothGATTService::GetServicesHelper(bt_gatt_h handle,
                                                       const std::string& address,
                                                       picojson::array* array) {
  LoggerD("Entered");

  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    return PlatformResult(ErrorCode::INVALID_STATE_ERR,
                          "Device is not connected");
  }

  int ret = bt_gatt_service_foreach_included_services(
      handle,
      [](int total, int index, bt_gatt_h gatt_handle, void *data) {
        LoggerD("Enter");

        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();

        char* uuid = nullptr;

        if (BT_ERROR_NONE == bt_gatt_get_uuid(gatt_handle, &uuid) && nullptr != uuid) {
          result_obj.insert(std::make_pair(kUuid, picojson::value(uuid)));
          free(uuid);
        } else {
          result_obj.insert(std::make_pair(kUuid, picojson::value("FFFF")));
        }

        //handle is passed to upper layer because there is no need of deletion
        result_obj.insert(std::make_pair(kHandle, picojson::value((double)(long)gatt_handle)));
        static_cast<picojson::array*>(data)->push_back(result);
        return true;
      }, array);
  if (BT_ERROR_NONE != ret) {
    LoggerE("Failed bt_gatt_service_foreach_included_services() (%d)", ret);
    return util::GetBluetoothError(ret, "Failed to set a service's GATT callback");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::GetCharacteristics(const picojson::value& args,
                                              picojson::object& out) {
  LoggerD("Entered");

  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());
  const std::string& uuid = args.get("uuid").get<std::string>();
  const std::string& address = args.get("address").get<std::string>();

  picojson::array array;
  PlatformResult ret = GetCharacteristicsHelper(handle, address, uuid, &array);
  if (ret.IsError()) {
    LoggerE("Error while getting characteristics");
    ReportError(ret, &out);
  } else {
    ReportSuccess(picojson::value(array), out);
  }
}

PlatformResult BluetoothGATTService::GetCharacteristicsHelper(bt_gatt_h handle,
                                                              const std::string& address,
                                                              const std::string& uuid,
                                                              picojson::array* array) {
  LoggerD("Entered");

  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    return PlatformResult(ErrorCode::INVALID_STATE_ERR,
                          "Device is not connected");
  }

  struct Data {
    picojson::array* array;
    PlatformResult* platform_res;
  };

  PlatformResult platform_result = PlatformResult(ErrorCode::NO_ERROR);
  Data user_data = {array, &platform_result};

  int ret = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h gatt_handle, void *data) {
        LoggerD("Enter");
        Data* user_data = static_cast<Data*>(data);
        picojson::array* array = user_data->array;
        PlatformResult* platform_result = user_data->platform_res;

        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();

        //handle is passed to upper layer because there is no need of deletion
        result_obj.insert(std::make_pair(kHandle, picojson::value((double)(long)gatt_handle)));

        //descriptors
        picojson::array& desc_array = result_obj.insert(
            std::make_pair("descriptors", picojson::value(picojson::array()))).
                first->second.get<picojson::array>();
        int ret = bt_gatt_characteristic_foreach_descriptors(
            gatt_handle,
            [](int total, int index, bt_gatt_h desc_handle, void *data) {
              LoggerD("Enter");
              picojson::array& desc_array = *(static_cast<picojson::array*>(data));

              picojson::value desc = picojson::value(picojson::object());
              picojson::object& desc_obj = desc.get<picojson::object>();

              //handle is passed to upper layer because there is no need of deletion
              desc_obj.insert(std::make_pair(kHandle, picojson::value(
                  (double)(long)desc_handle)));
              desc_array.push_back(desc);
              return true;
            }, static_cast<void*>(&desc_array));
        if (BT_ERROR_NONE != ret) {
          *platform_result = util::GetBluetoothError(ret, "Failed to get descriptors");
          LoggerE("Failed bt_gatt_characteristic_foreach_descriptors() (%d)", ret);
          return false;
        }

        //other properties
        int property_bits = 0;
        int err = bt_gatt_characteristic_get_properties(gatt_handle, &property_bits);
        if(BT_ERROR_NONE != err) {
          LoggerE("Properties of characteristic couldn't be acquired");
        }
        result_obj.insert(std::make_pair(kBroadcast, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_BROADCAST))));
        result_obj.insert(std::make_pair(kReadable, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_READ))));
        result_obj.insert(std::make_pair(kWriteNoResponse, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_WRITE_WITHOUT_RESPONSE))));
        result_obj.insert(std::make_pair(kWritable, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_WRITE))));
        result_obj.insert(std::make_pair(kNotify, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_NOTIFY))));
        result_obj.insert(std::make_pair(kIndication, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_INDICATE))));
        result_obj.insert(std::make_pair(kSignedWrite, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_AUTHENTICATED_SIGNED_WRITES))));
        result_obj.insert(std::make_pair(kExtendedProperties, picojson::value(
            IsProperty(property_bits, BT_GATT_PROPERTY_EXTENDED_PROPERTIES))));

        array->push_back(result);
        return true;
      }, static_cast<void*>(&user_data));
  if (platform_result.IsError()) {
    return platform_result;
  }
  if (BT_ERROR_NONE != ret) {
    LoggerE("Failed (%d)", ret);
    return util::GetBluetoothError(ret, "Failed while getting characteristic");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::ReadValue(const picojson::value& args,
                                     picojson::object& out) {
  LoggerD("Entered");

  const std::string& address = args.get("address").get<std::string>();
  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Device is not connected"), &out);
    return;
  }

  const double callback_handle = util::GetAsyncCallbackHandle(args);
  struct Data {
    double callback_handle;
    BluetoothGATTService* service;
  };

  Data* user_data = new Data{callback_handle, this};
  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());

  auto read_value = [](int result, bt_gatt_h handle, void *user_data) -> void {
    Data* data = static_cast<Data*>(user_data);
    double callback_handle = data->callback_handle;
    BluetoothGATTService* service = data->service;
    delete data;

    PlatformResult plarform_res = PlatformResult(ErrorCode::NO_ERROR);

    picojson::value byte_array = picojson::value(picojson::array());
    picojson::array& byte_array_obj = byte_array.get<picojson::array>();

    if (BT_ERROR_NONE != result) {
      plarform_res = util::GetBluetoothError(result, "Error while reading value");
    } else {
      char *value = nullptr;
      int length = 0;
      int ret = bt_gatt_get_value(handle, &value, &length);
      if (BT_ERROR_NONE != ret) {
        plarform_res = util::GetBluetoothError(ret, "Error while getting value");
      } else {
        for (int i = 0 ; i < length; i++) {
          byte_array_obj.push_back(picojson::value(std::to_string(value[i])));
        }
      }
      if (value) {
        free(value);
        value = nullptr;
      }
    }

    std::shared_ptr<picojson::value> response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    if (plarform_res.IsSuccess()) {
      ReportSuccess(byte_array, response->get<picojson::object>());
    } else {
      ReportError(plarform_res, &response->get<picojson::object>());
    }
    TaskQueue::GetInstance().Async<picojson::value>(
        [service, callback_handle](const std::shared_ptr<picojson::value>& response) {
      service->instance_.SyncResponse(callback_handle, response);
    }, response);
  };
  int ret = bt_gatt_client_read_value(handle, read_value, (void*)user_data);
  if (BT_ERROR_NONE != ret) {
    LOGE("Couldn't register callback for read value");
  }
  ReportSuccess(out);
}


void BluetoothGATTService::WriteValue(const picojson::value& args,
                                     picojson::object& out) {
  LoggerD("Entered");

  const std::string& address = args.get("address").get<std::string>();
  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Device is not connected"), &out);
    return;
  }

  const double callback_handle = util::GetAsyncCallbackHandle(args);
  const picojson::array& value_array = args.get("value").get<picojson::array>();

  int value_size = value_array.size();
  std::unique_ptr<char[]> value_data(new char[value_size]);
  for (int i = 0; i < value_size; ++i) {
    value_data[i] = static_cast<char>(value_array[i].get<double>());
  }

  struct Data {
    double callback_handle;
    BluetoothGATTService* service;
  };

  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());

  auto write_value = [](int result, bt_gatt_h handle, void *user_data) -> void {
    Data* data = static_cast<Data*>(user_data);
    double callback_handle = data->callback_handle;
    BluetoothGATTService* service = data->service;
    delete data;

    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);
    if (BT_ERROR_NONE != result) {
      ret = util::GetBluetoothError(result, "Error while getting value");
    }

    std::shared_ptr<picojson::value> response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    if (ret.IsSuccess()) {
      ReportSuccess(response->get<picojson::object>());
    } else {
      ReportError(ret, &response->get<picojson::object>());
    }
    TaskQueue::GetInstance().Async<picojson::value>(
        [service, callback_handle](const std::shared_ptr<picojson::value>& response) {
      service->instance_.SyncResponse(callback_handle, response);
    }, response);
  };

  int ret = bt_gatt_set_value(handle, value_data.get(), value_size);

  if (BT_ERROR_NONE != ret) {
    LoggerE("Couldn't set value");
    std::shared_ptr<picojson::value> response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    ReportError(util::GetBluetoothError(ret, "Failed to set value"),
                &response->get<picojson::object>());
    TaskQueue::GetInstance().Async<picojson::value>(
        [this, callback_handle](const std::shared_ptr<picojson::value>& response) {
      instance_.SyncResponse(callback_handle, response);
    }, response);
  } else {
    Data* user_data = new Data{callback_handle, this};
    ret = bt_gatt_client_write_value(handle, write_value, user_data);
    if (BT_ERROR_NONE != ret) {
      delete user_data;
      LoggerE("Couldn't register callback for write value");
    }
  }
  ReportSuccess(out);
}

void BluetoothGATTService::AddValueChangeListener(const picojson::value& args,
                                                  picojson::object& out) {
  LoggerD("Entered");
  const auto& address = args.get("address").get<std::string>();
  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Device is not connected"), &out);
    return;
  }

  bt_gatt_h handle = (bt_gatt_h)static_cast<long>(args.get(kHandle).get<double>());

  int ret = bt_gatt_client_set_characteristic_value_changed_cb(handle, OnCharacteristicValueChanged, this);
  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_gatt_client_set_characteristic_value_changed_cb() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to register listener"), &out);
  } else {
    gatt_characteristic_.push_back(handle);
    ReportSuccess(out);
  }
}

void BluetoothGATTService::RemoveValueChangeListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  const auto& address = args.get("address").get<std::string>();
  if (!IsStillConnected(address)) {
    LoggerE("Device with address %s is no longer connected", address.c_str());
    ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR,
                               "Device is not connected"), &out);
    return;
  }

  bt_gatt_h handle = (bt_gatt_h)static_cast<long>(args.get(kHandle).get<double>());

  int ret = bt_gatt_client_unset_characteristic_value_changed_cb(handle);

  if (BT_ERROR_NONE != ret) {
    LoggerE("bt_gatt_client_unset_characteristic_value_changed_cb() failed with: %d", ret);
    ReportError(util::GetBluetoothError(ret, "Failed to unregister listener"), &out);
  } else {
    gatt_characteristic_.erase(std::remove(gatt_characteristic_.begin(), gatt_characteristic_.end(), handle), gatt_characteristic_.end());
    ReportSuccess(out);
  }
}

common::PlatformResult BluetoothGATTService::GetServiceUuids(
    const std::string& address, picojson::array* array) {
  LoggerD("Entered");

  bt_gatt_client_h client = GetGattClient(address);

  if (nullptr == client) {
    return PlatformResult(ErrorCode::UNKNOWN_ERR, "Unable to create client");
  }

  auto foreach_callback = [](int total, int index, bt_gatt_h gatt_handle, void* user_data) -> bool {
    LoggerD("Entered foreach_callback, total: %d, index: %d", total, index);

    char* uuid = nullptr;
    int ret = bt_gatt_get_uuid(gatt_handle, &uuid);

    if (BT_ERROR_NONE != ret || nullptr == uuid) {
      LoggerE("Failed to get UUID: %d", ret);
    } else {
      std::string u = std::string(uuid);
      free(uuid);
      if (u.length() > 4) {  // 128-bit UUID, needs to be converted to 16-bit
        u = u.substr(4, 4);
      }
      static_cast<picojson::array*>(user_data)->push_back(picojson::value(u));
    }

    return true;
  };

  int ret = bt_gatt_client_foreach_services(client, foreach_callback, array);

  if (BT_ERROR_NONE == ret) {
    return PlatformResult(ErrorCode::NO_ERROR);
  } else {
    LoggerE("Failed to get UUIDS: %d", ret);
    return util::GetBluetoothError(ret, "Failed to get UUIDS");
  }
}

void BluetoothGATTService::OnCharacteristicValueChanged(
    bt_gatt_h characteristic, char* value, int length, void* user_data) {
  LoggerD("Entered, characteristic: [%p], len: [d], user_data: [%p]", characteristic, length, user_data);

  auto service = static_cast<BluetoothGATTService*>(user_data);

  if (!service) {
    LoggerE("user_data is NULL");
    return;
  }

  picojson::value result = picojson::value(picojson::object());
  picojson::object result_obj = result.get<picojson::object>();

  result_obj.insert(std::make_pair(kHandle, picojson::value((double)(long)characteristic)));

  picojson::value byte_array = picojson::value(picojson::array());
  picojson::array& byte_array_obj = byte_array.get<picojson::array>();

  for (int i = 0 ; i < length; ++i) {
    byte_array_obj.push_back(picojson::value(std::to_string(value[i])));
  }

  ReportSuccess(byte_array, result_obj);

  service->instance_.FireEvent(kOnValueChanged, result);
}

} // namespace bluetooth
} // namespace extension
