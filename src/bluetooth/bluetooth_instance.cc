
// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "bluetooth_instance.h"

#include "common/converter.h"
#include "common/logger.h"

#include "bluetooth_adapter.h"
#include "bluetooth_device.h"
#include "bluetooth_health_application.h"
#include "bluetooth_health_channel.h"
#include "bluetooth_health_profile_handler.h"
#include "bluetooth_service_handler.h"
#include "bluetooth_socket.h"
#include "bluetooth_util.h"

namespace extension {
namespace bluetooth {

using namespace common;

namespace {

BluetoothAdapter* bluetooth_adapter = &BluetoothAdapter::GetInstance();
BluetoothDevice bluetooth_device;
BluetoothHealthApplication bluetooth_health_application;
BluetoothHealthChannel bluetooth_health_channel;
BluetoothHealthProfileHandler* bluetooth_health_profile_handler = &BluetoothHealthProfileHandler::GetInstance();
BluetoothServiceHandler bluetooth_service_handler;
BluetoothSocket bluetooth_socket;

void CheckPrivilege(const picojson::value& data, picojson::object& out)
{
    const auto& args = util::GetArguments(data);
    const auto& privilege = FromJson<std::string>(args, "privilege");
    util::CheckAccess(privilege);

    tools::ReportSuccess(out);
}

} // namespace

BluetoothInstance& BluetoothInstance::GetInstance()
{
    static BluetoothInstance instance;
    return instance;
}

BluetoothInstance::BluetoothInstance()
{
    LoggerD("Entered");
    using namespace std::placeholders;

    // BluetoothAdapter
    RegisterHandler("BluetoothAdapter_setName",
                    std::bind(&BluetoothAdapter::SetName, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_setPowered",
                    std::bind(&BluetoothAdapter::SetPowered, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_setVisible",
                    std::bind(&BluetoothAdapter::SetVisible, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_discoverDevices",
                        std::bind(&BluetoothAdapter::DiscoverDevices, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_stopDiscovery",
                    std::bind(&BluetoothAdapter::StopDiscovery, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_getKnownDevices",
                    std::bind(&BluetoothAdapter::GetKnownDevices, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_getDevice",
                    std::bind(&BluetoothAdapter::GetDevice, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_createBonding",
                    std::bind(&BluetoothAdapter::CreateBonding, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_destroyBonding",
                    std::bind(&BluetoothAdapter::DestroyBonding, bluetooth_adapter, _1, _2));
    RegisterHandler("BluetoothAdapter_registerRFCOMMServiceByUUID",
                    std::bind(&BluetoothAdapter::RegisterRFCOMMServiceByUUID, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_getBluetoothProfileHandler",
                        std::bind(&BluetoothAdapter::GetBluetoothProfileHandler, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_getName",
                        std::bind(&BluetoothAdapter::GetName, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_getAddress",
                        std::bind(&BluetoothAdapter::GetAddress, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_getPowered",
                        std::bind(&BluetoothAdapter::GetPowered, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_getVisible",
                        std::bind(&BluetoothAdapter::GetVisible, bluetooth_adapter, _1, _2));
    RegisterSyncHandler("BluetoothAdapter_isServiceConnected",
                        std::bind(&BluetoothAdapter::IsServiceConnected, bluetooth_adapter, _1, _2));

    // BluetoothDevice
    RegisterHandler("BluetoothDevice_connectToServiceByUUID",
                    std::bind(&BluetoothDevice::ConnectToServiceByUUID, &bluetooth_device, _1, _2));
    RegisterSyncHandler("BluetoothDevice_getBoolValue",
                        std::bind(&BluetoothDevice::GetBoolValue, &bluetooth_device, _1, _2));

    // BluetoothHealthApplication
    RegisterHandler("BluetoothHealthApplication_unregister",
                    std::bind(&BluetoothHealthApplication::Unregister, &bluetooth_health_application, _1, _2));

    // BluetoothHealthChannel
    RegisterSyncHandler("BluetoothHealthChannel_close",
                        std::bind(&BluetoothHealthChannel::Close, &bluetooth_health_channel, _1, _2));
    RegisterSyncHandler("BluetoothHealthChannel_sendData",
                        std::bind(&BluetoothHealthChannel::SendData, &bluetooth_health_channel, _1, _2));

    // BluetoothHealthProfileHandler
    RegisterHandler("BluetoothHealthProfileHandler_registerSinkApp",
                    std::bind(&BluetoothHealthProfileHandler::RegisterSinkApp, bluetooth_health_profile_handler, _1, _2));
    RegisterHandler("BluetoothHealthProfileHandler_connectToSource",
                    std::bind(&BluetoothHealthProfileHandler::ConnectToSource, bluetooth_health_profile_handler, _1, _2));

    // BluetoothServiceHandler
    RegisterHandler("BluetoothServiceHandler_unregister",
                    std::bind(&BluetoothServiceHandler::Unregister, &bluetooth_service_handler, _1, _2));

    // BluetoothSocket
    RegisterSyncHandler("BluetoothSocket_writeData",
                        std::bind(&BluetoothSocket::WriteData, &bluetooth_socket, _1, _2));
    RegisterSyncHandler("BluetoothSocket_readData",
                        std::bind(&BluetoothSocket::ReadData, &bluetooth_socket, _1, _2));
    RegisterSyncHandler("BluetoothSocket_close",
                        std::bind(&BluetoothSocket::Close, &bluetooth_socket, _1, _2));

    // other
    RegisterSyncHandler("Bluetooth_checkPrivilege", CheckPrivilege);
}

BluetoothInstance::~BluetoothInstance()
{
    LoggerD("Entered");
}

} // namespace bluetooth
} // namespace extension

