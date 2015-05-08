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

const std::string kDescriptors = "descriptors";
const std::string kBroadcast = "isBroadcast";
const std::string kExtendedProperties = "hasExtendedProperties";
const std::string kNotify = "isNotify";
const std::string kIndication = "isIndication";
const std::string kReadable = "isReadable";
const std::string kSignedWrite = "isSignedWrite";
const std::string kWritable = "isWritable";
const std::string kWriteNoResponse = "isWriteNoResponse";


bool IsProperty (int propertyBits, bt_gatt_property_e property) {
  return (propertyBits & property) == 0;
}
}

BluetoothGATTService::BluetoothGATTService(BluetoothInstance& instance) :
        instance_(instance)
{
  LoggerD("Entered");
}

BluetoothGATTService::~BluetoothGATTService() {
  LoggerD("Entered");

  for (auto it : gatt_clients_) {
    LoggerD("destroying client for address: %s", it.first.c_str());
    bt_gatt_client_destroy(it.second);
  }
}

PlatformResult BluetoothGATTService::GetSpecifiedGATTService(const std::string &address,
                                                             const std::string &uuid,
                                                             picojson::object* result) {
  LoggerD("Entered");

  bt_gatt_client_h client = nullptr;
  auto it = gatt_clients_.find(address);
  int ret = BT_ERROR_NONE;
  if (gatt_clients_.end() == it) {
    ret = bt_gatt_client_create(address.c_str(), &client);
    if (BT_ERROR_NONE != ret) {
      LoggerE("%d", ret);
      //TODO check error code
      return PlatformResult(ErrorCode::UNKNOWN_ERR,
                            "Failed to create the GATT client's handle");
    }
    gatt_clients_.insert(std::make_pair(address, client));
  } else {
    LoggerD("Client already created");
    client = it->second;
  }

  bt_gatt_h service = nullptr;
  ret = bt_gatt_client_get_service(client, uuid.c_str(), &service);
  if (BT_ERROR_NONE != ret) {
    LoggerE("%d", ret);
    //TODO check error code
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to get a service's GATT handle");
  }

  //report BluetoothGattService
  result->insert(std::make_pair(kUuid, picojson::value(uuid)));
  //handle is passed to upper layer because there is no need to delete it
  result->insert(std::make_pair(kHandle, picojson::value((double)(long)service)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::GetServices(const picojson::value& args,
                                       picojson::object& out) {
  LoggerD("Entered");

  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());
  std::string uuid = args.get("uuid").get<std::string>();

  picojson::array array;
  PlatformResult ret = GetServicesHelper(handle, uuid, &array);
  if (ret.IsError()) {
    LoggerE("Error while getting services");
    ReportError(ret, &out);
  } else {
    ReportSuccess(picojson::value(array), out);
  }
}

PlatformResult BluetoothGATTService::GetServicesHelper(bt_gatt_h handle,
                                                       const std::string& uuid,
                                                       picojson::array* array) {
  LoggerD("Entered");

  struct Data {
    const std::string& uuid;
    picojson::array* array;
  };
  Data user_data {uuid, array};

  int ret = bt_gatt_service_foreach_included_services(
      handle,
      [](int total, int index, bt_gatt_h gatt_handle, void *data) {
        LoggerD("Enter");
        Data user_data = *(static_cast<Data*>(data));

        picojson::value result = picojson::value(picojson::object());
        picojson::object& result_obj = result.get<picojson::object>();

        result_obj.insert(std::make_pair(kUuid, picojson::value(user_data.uuid)));
        //handle is passed to upper layer because there is no need of deletion
        result_obj.insert(std::make_pair(kHandle, picojson::value((double)(long)gatt_handle)));
        user_data.array->push_back(result);
        return true;
      }, static_cast<void*>(&user_data));
  if (BT_ERROR_NONE != ret) {
    //TODO check error code
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed to set a service's GATT callback");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::GetCharacteristics(const picojson::value& args,
                                              picojson::object& out) {
  LoggerD("Entered");

  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());
  std::string uuid = args.get("uuid").get<std::string>();

  picojson::array array;
  PlatformResult ret = GetCharacteristicsHelper(handle, uuid, &array);
  if (ret.IsError()) {
    LoggerE("Error while getting characteristics");
    ReportError(ret, &out);
  } else {
    ReportSuccess(picojson::value(array), out);
  }
}

PlatformResult BluetoothGATTService::GetCharacteristicsHelper(bt_gatt_h handle,
                                                              const std::string& uuid,
                                                              picojson::array* array) {
  LoggerD("Entered");

  int ret = bt_gatt_service_foreach_characteristics(
      handle,
      [](int total, int index, bt_gatt_h gatt_handle, void *data) {
        LoggerD("Enter");
        picojson::array* array = static_cast<picojson::array*>(data);

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
          //TODO check error code
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
      }, static_cast<void*>(&array));
  if (BT_ERROR_NONE != ret) {
    //TODO check error code
    return PlatformResult(ErrorCode::UNKNOWN_ERR,
                          "Failed while getting characteristic");
  }

  return PlatformResult(ErrorCode::NO_ERROR);
}

void BluetoothGATTService::ReadValue(const picojson::value& args,
                                     picojson::object& out) {
  LoggerD("Entered");
  const double callback_handle = util::GetAsyncCallbackHandle(args);
  struct Data {
    double callback_handle;
    BluetoothGATTService* service;
  };

  Data* user_data = new Data{callback_handle, this};
  bt_gatt_h handle = (bt_gatt_h) static_cast<long>(args.get("handle").get<double>());

  //TODO check if client device is still connected and throw InvalidStateError

  auto read_value = [](int result, bt_gatt_h handle, void *user_data) -> void {
    Data* data = (Data*) user_data;
    double callback_handle = data->callback_handle;
    BluetoothGATTService* service = data->service;
    delete data;

    PlatformResult ret = PlatformResult(ErrorCode::NO_ERROR);

    picojson::value byte_array = picojson::value(picojson::array());
    picojson::array& byte_array_obj = byte_array.get<picojson::array>();

    if (BT_ERROR_NONE != result) {
      //TODO handle error
    } else {
      char *value = nullptr;
      int length = 0;
      int ret = bt_gatt_get_value(handle, &value, &length);
      if (BT_ERROR_NONE != result) {
        //TODO handle error
      }

      for (size_t i = 0 ; i < length; i++) {
        byte_array_obj.push_back(picojson::value(std::to_string(value[i])));
      }
      if (value) {
        free(value);
        value = nullptr;
      }
    }

    std::shared_ptr<picojson::value> response =
        std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));
    if (ret.IsSuccess()) {
      ReportSuccess(byte_array, response->get<picojson::object>());
    } else {
      ReportError(ret, &response->get<picojson::object>());
    }
    TaskQueue::GetInstance().Async<picojson::value>(
        [service, callback_handle](const std::shared_ptr<picojson::value>& response) {
      service->instance_.SyncResponse(callback_handle, response);
    }, response);
  };
  int ret = bt_gatt_client_read_value(handle, read_value, (void*)user_data);
  if (BT_ERROR_NONE != ret) {
    LOGE("Couldn't register callback for read value");
    //TODO handle error ??
  }
  ReportSuccess(out);
}

} // namespace bluetooth
} // namespace extension
