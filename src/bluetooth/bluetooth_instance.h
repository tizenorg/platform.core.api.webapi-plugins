// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLUETOOTH_BLUETOOTH_INSTANCE_H_
#define BLUETOOTH_BLUETOOTH_INSTANCE_H_

#include "common/extension.h"

#include "bluetooth/bluetooth_adapter.h"
#include "bluetooth/bluetooth_device.h"
#include "bluetooth/bluetooth_health_application.h"
#include "bluetooth/bluetooth_health_channel.h"
#include "bluetooth/bluetooth_health_profile_handler.h"
#include "bluetooth/bluetooth_le_adapter.h"
#include "bluetooth/bluetooth_service_handler.h"
#include "bluetooth/bluetooth_socket.h"
#include "bluetooth/bluetooth_util.h"

namespace extension {
namespace bluetooth {

class BluetoothInstance: public common::ParsedInstance {
 public:
  BluetoothInstance();
  virtual ~BluetoothInstance();

  void AsyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response);
  void AsyncResponse(double callback_handle, const common::PlatformResult& result);
  void SyncResponse(double callback_handle, const std::shared_ptr<picojson::value>& response);

  void FireEvent(const std::string& event, picojson::value& value);
  void FireEvent(const std::string& event, const picojson::value& value);
  void FireEvent(const std::string& event, const std::shared_ptr<picojson::value>& value);

 private:
  BluetoothAdapter bluetooth_adapter_;
  BluetoothDevice bluetooth_device_;
  BluetoothHealthApplication bluetooth_health_application_;
  BluetoothHealthChannel bluetooth_health_channel_;
  BluetoothHealthProfileHandler bluetooth_health_profile_handler_;
  BluetoothServiceHandler bluetooth_service_handler_;
  BluetoothSocket bluetooth_socket_;
  BluetoothLEAdapter bluetooth_le_adapter_;
};

} // namespace bluetooth
} // namespace extension

#endif // BLUETOOTH_BLUETOOTH_INSTANCE_H_
