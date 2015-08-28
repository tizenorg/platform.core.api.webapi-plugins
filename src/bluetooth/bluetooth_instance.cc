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

#include "bluetooth_instance.h"

#include "common/converter.h"
#include "common/logger.h"
#include "common/task-queue.h"

namespace extension {
namespace bluetooth {

using namespace common;

BluetoothInstance::BluetoothInstance() :
    bluetooth_adapter_(*this),
    bluetooth_device_(bluetooth_adapter_),
    bluetooth_health_application_(bluetooth_health_profile_handler_),
    bluetooth_health_profile_handler_(*this),
    bluetooth_service_handler_(bluetooth_adapter_),
    bluetooth_socket_(bluetooth_adapter_),
    bluetooth_le_adapter_(*this),
    bluetooth_gatt_service_(*this),
    bluetooth_le_device_(*this, bluetooth_gatt_service_)
{
  LoggerD("Entered");
  using std::placeholders::_1;
  using std::placeholders::_2;

  #define REGISTER_ASYNC(c, func) \
      RegisterSyncHandler(c, func);
  #define REGISTER_SYNC(c, func) \
      RegisterSyncHandler(c, func);

  // BluetoothAdapter
  REGISTER_ASYNC("BluetoothAdapter_setName",
      std::bind(&BluetoothAdapter::SetName, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_setPowered",
      std::bind(&BluetoothAdapter::SetPowered, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_setVisible",
      std::bind(&BluetoothAdapter::SetVisible, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_discoverDevices",
      std::bind(&BluetoothAdapter::DiscoverDevices, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_stopDiscovery",
      std::bind(&BluetoothAdapter::StopDiscovery, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_getKnownDevices",
      std::bind(&BluetoothAdapter::GetKnownDevices, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_getDevice",
      std::bind(&BluetoothAdapter::GetDevice, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_createBonding",
      std::bind(&BluetoothAdapter::CreateBonding, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_destroyBonding",
      std::bind(&BluetoothAdapter::DestroyBonding, &bluetooth_adapter_, _1, _2));
  REGISTER_ASYNC("BluetoothAdapter_registerRFCOMMServiceByUUID",
      std::bind(&BluetoothAdapter::RegisterRFCOMMServiceByUUID, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_getBluetoothProfileHandler",
      std::bind(&BluetoothAdapter::GetBluetoothProfileHandler, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_getName",
      std::bind(&BluetoothAdapter::GetName, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_getAddress",
      std::bind(&BluetoothAdapter::GetAddress, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_getPowered",
      std::bind(&BluetoothAdapter::GetPowered, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_getVisible",
      std::bind(&BluetoothAdapter::GetVisible, &bluetooth_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothAdapter_isServiceConnected",
      std::bind(&BluetoothAdapter::IsServiceConnected, &bluetooth_adapter_, _1, _2));

  // BluetoothDevice
  REGISTER_ASYNC("BluetoothDevice_connectToServiceByUUID",
      std::bind(&BluetoothDevice::ConnectToServiceByUUID, &bluetooth_device_, _1, _2));
  REGISTER_SYNC("BluetoothDevice_getBoolValue",
      std::bind(&BluetoothDevice::GetBoolValue, &bluetooth_device_, _1, _2));

  // BluetoothHealthApplication
  REGISTER_ASYNC("BluetoothHealthApplication_unregister",
      std::bind(&BluetoothHealthApplication::Unregister, &bluetooth_health_application_, _1, _2));

  // BluetoothHealthChannel
  REGISTER_SYNC("BluetoothHealthChannel_close",
      std::bind(&BluetoothHealthChannel::Close, &bluetooth_health_channel_, _1, _2));
  REGISTER_SYNC("BluetoothHealthChannel_sendData",
      std::bind(&BluetoothHealthChannel::SendData, &bluetooth_health_channel_, _1, _2));

  // BluetoothHealthProfileHandler
  REGISTER_ASYNC("BluetoothHealthProfileHandler_registerSinkApp",
      std::bind(&BluetoothHealthProfileHandler::RegisterSinkApp, &bluetooth_health_profile_handler_, _1, _2));
  REGISTER_ASYNC("BluetoothHealthProfileHandler_connectToSource",
      std::bind(&BluetoothHealthProfileHandler::ConnectToSource, &bluetooth_health_profile_handler_, _1, _2));

  // BluetoothServiceHandler
  REGISTER_ASYNC("BluetoothServiceHandler_unregister",
      std::bind(&BluetoothServiceHandler::Unregister, &bluetooth_service_handler_, _1, _2));

  // BluetoothSocket
  REGISTER_SYNC("BluetoothSocket_writeData",
      std::bind(&BluetoothSocket::WriteData, &bluetooth_socket_, _1, _2));
  REGISTER_SYNC("BluetoothSocket_readData",
      std::bind(&BluetoothSocket::ReadData, &bluetooth_socket_, _1, _2));
  REGISTER_SYNC("BluetoothSocket_close",
      std::bind(&BluetoothSocket::Close, &bluetooth_socket_, _1, _2));

  // BluetoothLEAdapter
  REGISTER_SYNC("BluetoothLEAdapter_startScan",
      std::bind(&BluetoothLEAdapter::StartScan, &bluetooth_le_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothLEAdapter_stopScan",
      std::bind(&BluetoothLEAdapter::StopScan, &bluetooth_le_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothLEAdapter_startAdvertise",
      std::bind(&BluetoothLEAdapter::StartAdvertise, &bluetooth_le_adapter_, _1, _2));
  REGISTER_SYNC("BluetoothLEAdapter_stopAdvertise",
      std::bind(&BluetoothLEAdapter::StopAdvertise, &bluetooth_le_adapter_, _1, _2));

  // BluetoothLEDevice
  REGISTER_ASYNC(
      "BluetoothLEDevice_connect",
      std::bind(&BluetoothLEDevice::Connect, &bluetooth_le_device_, _1, _2));
  REGISTER_ASYNC(
      "BluetoothLEDevice_disconnect",
      std::bind(&BluetoothLEDevice::Disconnect, &bluetooth_le_device_, _1, _2));
  REGISTER_SYNC(
      "BluetoothLEDevice_getService",
      std::bind(&BluetoothLEDevice::GetService, &bluetooth_le_device_, _1, _2));
  REGISTER_SYNC(
      "BluetoothLEDevice_addConnectStateChangeListener",
      std::bind(&BluetoothLEDevice::AddConnectStateChangeListener,
                &bluetooth_le_device_, _1, _2));
  REGISTER_SYNC(
      "BluetoothLEDevice_removeConnectStateChangeListener",
      std::bind(&BluetoothLEDevice::RemoveConnectStateChangeListener,
                &bluetooth_le_device_, _1, _2));
  REGISTER_SYNC(
        "BluetoothLEDevice_getServiceUuids",
        std::bind(&BluetoothLEDevice::GetServiceUuids,
                  &bluetooth_le_device_, _1, _2));

  // BluetoothGATTService
  REGISTER_SYNC("BluetoothGATTService_getServices",
      std::bind(&BluetoothGATTService::GetServices, &bluetooth_gatt_service_, _1, _2));
  REGISTER_SYNC("BluetoothGATTService_getCharacteristics",
      std::bind(&BluetoothGATTService::GetCharacteristics, &bluetooth_gatt_service_, _1, _2));
  REGISTER_SYNC("BluetoothGATT_readValue",
      std::bind(&BluetoothGATTService::ReadValue, &bluetooth_gatt_service_, _1, _2));
  REGISTER_SYNC("BluetoothGATT_writeValue",
      std::bind(&BluetoothGATTService::WriteValue, &bluetooth_gatt_service_, _1, _2));
  REGISTER_SYNC(
      "BluetoothGATTCharacteristic_addValueChangeListener",
      std::bind(&BluetoothGATTService::AddValueChangeListener,
                &bluetooth_gatt_service_, _1, _2));
  REGISTER_SYNC(
      "BluetoothGATTCharacteristic_removeValueChangeListener",
      std::bind(&BluetoothGATTService::RemoveValueChangeListener,
                &bluetooth_gatt_service_, _1, _2));

  #undef REGISTER_ASYNC
  #undef REGISTER_SYNC
}

BluetoothInstance::~BluetoothInstance()
{
  LoggerD("Entered");
}

namespace {
const char* JSON_CALLBACK_ID = "callbackId";
const char* JSON_LISTENER_ID = "listenerId";
} // namespace

void BluetoothInstance::AsyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response) {
  LoggerD("Entered");
  common::TaskQueue::GetInstance().Async<picojson::value>([this, callback_handle](const std::shared_ptr<picojson::value>& response) {
    SyncResponse(callback_handle, response);
  }, response);
}

void BluetoothInstance::AsyncResponse(double callback_handle, const PlatformResult& result) {
  LoggerD("Entered");
  std::shared_ptr<picojson::value> response =
      std::shared_ptr<picojson::value>(new picojson::value(picojson::object()));

  if (result.IsError()) {
    tools::ReportError(result, &response->get<picojson::object>());
  } else {
    tools::ReportSuccess(response->get<picojson::object>());
  }

  TaskQueue::GetInstance().Async<picojson::value>([this, callback_handle](const std::shared_ptr<picojson::value>& response) {
    SyncResponse(callback_handle, response);
  }, response);
}

void BluetoothInstance::SyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response) {
  LoggerD("Entered");
  auto& obj = response->get<picojson::object>();
  obj[JSON_CALLBACK_ID] = picojson::value(callback_handle);
  Instance::PostMessage(this, response->serialize().c_str());
}

void BluetoothInstance::FireEvent(const std::string& event, picojson::value& value) {
  LoggerD("Entered");
  auto& obj = value.get<picojson::object>();
  obj[JSON_LISTENER_ID] = picojson::value(event);
  Instance::PostMessage(this, value.serialize().c_str());
}

void BluetoothInstance::FireEvent(const std::string& event, const picojson::value& value) {
  LoggerD("Entered");
  picojson::value v{value};
  FireEvent(event, v);
}

void BluetoothInstance::FireEvent(const std::string& event, const std::shared_ptr<picojson::value>& value) {
  LoggerD("Entered");
  FireEvent(event, *value.get());
}


} // namespace bluetooth
} // namespace extension

