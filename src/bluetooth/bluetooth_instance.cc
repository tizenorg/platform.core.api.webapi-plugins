
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
    bluetooth_health_profile_handler_(*this),
    bluetooth_health_application_(bluetooth_health_profile_handler_),
    bluetooth_service_handler_(bluetooth_adapter_),
    bluetooth_socket_(bluetooth_adapter_)
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
const char* JSON_STATUS = "status";
const char* JSON_RESULT = "result";
const char* JSON_CALLBACK_SUCCCESS = "success";
const char* JSON_CALLBACK_ERROR = "error";
const char* JSON_DATA = "args";
} // namespace

void BluetoothInstance::AsyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response) {
  common::TaskQueue::GetInstance().Async<picojson::value>([this, callback_handle](const std::shared_ptr<picojson::value>& response) {
    SyncResponse(callback_handle, response);
  }, response);
}

void BluetoothInstance::AsyncResponse(double callback_handle, const PlatformResult& result) {
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
  auto& obj = response->get<picojson::object>();
  obj[JSON_CALLBACK_ID] = picojson::value(callback_handle);
  PostMessage(response->serialize().c_str());
}

void BluetoothInstance::FireEvent(const std::string& event, picojson::value& value) {
  auto& obj = value.get<picojson::object>();
  obj[JSON_LISTENER_ID] = picojson::value(event);
  PostMessage(value.serialize().c_str());
}

void BluetoothInstance::FireEvent(const std::string& event, const picojson::value& value) {
  picojson::value v{value};
  FireEvent(event, v);
}

void BluetoothInstance::FireEvent(const std::string& event, const std::shared_ptr<picojson::value>& value) {
  FireEvent(event, *value.get());
}


} // namespace bluetooth
} // namespace extension

