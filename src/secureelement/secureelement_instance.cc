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

#include "secureelement/secureelement_instance.h"

#include <SEService.h>
#include <Reader.h>
#include "common/picojson.h"
#include "common/logger.h"
#include "common/task-queue.h"

#include "secureelement_reader.h"
#include "secureelement_session.h"
#include "secureelement_channel.h"

namespace extension {
namespace secureelement {

using namespace common;
using namespace smartcard_service_api;

SecureElementInstance::SecureElementInstance()
    : service_(*this) {
    LoggerD("Entered");

    using std::placeholders::_1;
    using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&SecureElementInstance::x, this, _1, _2));

    REGISTER_SYNC("SEService_registerSEListener", RegisterSEListener);
    REGISTER_SYNC("SEService_unregisterSEListener", UnregisterSEListener);
    REGISTER_SYNC("SEService_shutdown", Shutdown);
    REGISTER_SYNC("SEReader_getName", GetName);
    REGISTER_SYNC("SEReader_isPresent", IsPresent);
    REGISTER_SYNC("SEReader_closeSessions", CloseSessions);
    REGISTER_SYNC("SESession_getATR", GetATR);
    REGISTER_SYNC("SESession_isClosed", IsSessionClosed);
    REGISTER_SYNC("SESession_close", CloseSession);
    REGISTER_SYNC("SESession_closeChannels", CloseChannels);
    REGISTER_SYNC("SEChannel_close", CloseChannel);
    REGISTER_SYNC("SEChannel_getSelectResponse", GetSelectResponse);
#undef REGISTER_SYNC

#define REGISTER(c,x) \
    RegisterSyncHandler(c, std::bind(&SecureElementInstance::x, this, _1, _2));

    REGISTER("SEService_getReaders", GetReaders);
    REGISTER("SEReader_openSession", OpenSession);
    REGISTER("SESession_openBasicChannel", OpenBasicChannel);
    REGISTER("SESession_openLogicalChannel ", OpenLogicalChannel);
    REGISTER("SEChannel_transmit", Transmit);
#undef REGISTER
}

SecureElementInstance::~SecureElementInstance() {
}

void SecureElementInstance::GetReaders(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");

  double callback_id = 0.0;
  if (args.contains("callbackId")) {
    callback_id = args.get("callbackId").get<double>();
  }

  service_.GetReaders(callback_id);
  ReportSuccess(out);
}

void SecureElementInstance::RegisterSEListener(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.RegisterSEListener();
  ReportSuccess(out);
}

void SecureElementInstance::UnregisterSEListener(
    const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.UnregisterSEListener();
  ReportSuccess(out);
}

void SecureElementInstance::Shutdown(const picojson::value& args, picojson::object& out) {
  LoggerD("Entered");
  service_.Shutdown();
  ReportSuccess(out);
}

void SecureElementInstance::GetName(
        const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    Reader* reader_ptr = (Reader*) static_cast<long>(args.get("handle").get<double>());
    SEReader seReader(reader_ptr);
    picojson::value result = seReader.getName();
    ReportSuccess(result, out);
}

void SecureElementInstance::IsPresent(
        const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");

    Reader* reader_ptr = (Reader*) static_cast<long>(args.get("handle").get<double>());
    SEReader seReader(reader_ptr);
    picojson::value result = seReader.isPresent();
    ReportSuccess(result, out);
}

void SecureElementInstance::CloseSessions(
        const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    Reader* reader_ptr = (Reader*) static_cast<long>(args.get("handle").get<double>());
    SEReader seReader(reader_ptr);
    seReader.closeSessions();
    ReportSuccess(out);
}

void SecureElementInstance::CloseChannel( const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    ClientChannel* channel_ptr = (ClientChannel*) static_cast<long>(args.get("handle").get<double>());
    SEChannel seChannel(channel_ptr);
    seChannel.close();
    ReportSuccess(out);
}

void SecureElementInstance::GetSelectResponse( const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    ClientChannel* channel_ptr = (ClientChannel*) static_cast<long>(args.get("handle").get<double>());
    SEChannel seChannel(channel_ptr);

    ByteArray select_response = seChannel.getSelectResponse();
    picojson::value result = picojson::value(picojson::array());
    picojson::array& arr = result.get<picojson::array>();
    size_t select_response_size = select_response.size();
    for (size_t i = 0; i < select_response_size; i++) {
        arr.push_back(picojson::value(static_cast<double>(select_response[i])));
    }
    ReportSuccess( result, out);
}

void SecureElementInstance::OpenSession(
        const picojson::value& args, picojson::object& out) {
    LoggerD("Entered");
    const double callback_id = args.get("callbackId").get<double>();
    Reader* reader_ptr = (Reader*) static_cast<long>(args.get("handle").get<double>());

    auto open_session = [this, reader_ptr](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Opening session");
        try {
            SEReader seReader(reader_ptr);
            picojson::value result = seReader.openSession();
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const ErrorIO& err) {
            LoggerD("Library reported ErrorIO!");
            ReportError(PlatformResult(ErrorCode::IO_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalState& err) {
            LoggerD("Library reported ErrorIllegalState!");
            ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalParameter& err) {
            LoggerD("Library reported ErrorIllegalParameter!");
            ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR), &response->get<picojson::object>());
        } catch (const ErrorSecurity& err) {
            LoggerD("Library reported ErrorSecurity!");
            ReportError(PlatformResult(ErrorCode::SECURITY_ERR), &response->get<picojson::object>());
        } catch (const ExceptionBase& err) {
            LoggerD("Library reported ExceptionBase!");
            ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &response->get<picojson::object>());
        }
    };

    auto open_session_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");

        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
        Instance::PostMessage(this, response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>(
            open_session,
            open_session_response,
            std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

// Session functions

void SecureElementInstance::OpenBasicChannel( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    const double callback_id = args.get("callbackId").get<double>();
    const picojson::array v_aid = args.get("aid").get<picojson::value::array>();
    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());

    auto open = [this, v_aid, session_ptr](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Opening basic channel");
        try {
            SESession seSession(session_ptr);
            picojson::value result = seSession.openBasicChannel(v_aid);
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const ErrorIO& err) {
            LoggerD("Library reported ErrorIO!");
            ReportError(PlatformResult(ErrorCode::IO_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalState& err) {
            LoggerD("Library reported ErrorIllegalState!");
            ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalParameter& err) {
            LoggerD("Library reported ErrorIllegalParameter!");
            ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR), &response->get<picojson::object>());
        } catch (const ErrorSecurity& err) {
            LoggerD("Library reported ErrorSecurity!");
            ReportError(PlatformResult(ErrorCode::SECURITY_ERR), &response->get<picojson::object>());
        } catch (const ExceptionBase& err) {
            LoggerD("Library reported ExceptionBase!");
            ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
        Instance::PostMessage(this, response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        ( open, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}


void SecureElementInstance::OpenLogicalChannel( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    const double callback_id = args.get("callbackId").get<double>();
    const picojson::array v_aid = args.get("aid").get<picojson::value::array>();
    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());

    auto open = [this, v_aid, session_ptr](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Opening basic channel");
        try {
            SESession seSession(session_ptr);
            picojson::value result = seSession.openLogicalChannel(v_aid);
            ReportSuccess(result, response->get<picojson::object>());
        } catch (const ErrorIO& err) {
            LoggerD("Library reported ErrorIO!");
            ReportError(PlatformResult(ErrorCode::IO_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalState& err) {
            LoggerD("Library reported ErrorIllegalState!");
            ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalParameter& err) {
            LoggerD("Library reported ErrorIllegalParameter!");
            ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR), &response->get<picojson::object>());
        } catch (const ErrorSecurity& err) {
            LoggerD("Library reported ErrorSecurity!");
            ReportError(PlatformResult(ErrorCode::SECURITY_ERR), &response->get<picojson::object>());
        } catch (const ExceptionBase& err) {
            LoggerD("Library reported ExceptionBase!");
            ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
        Instance::PostMessage(this, response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        ( open, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}


void SecureElementInstance::GetATR( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");

    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());
    SESession seSession(session_ptr);

    ByteArray atr_result = seSession.getATR();
    picojson::value result = picojson::value(picojson::array());
    picojson::array& arr = result.get<picojson::array>();
    size_t atr_result_size = atr_result.size();
    for (size_t i = 0; i < atr_result_size; i++) {
        arr.push_back(picojson::value(static_cast<double>(atr_result[i])));
    }
    ReportSuccess( result, out);
}


void SecureElementInstance::IsSessionClosed( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());
    SESession seSession(session_ptr);
    picojson::value result = seSession.isClosed();
    ReportSuccess( result, out);
}


void SecureElementInstance::CloseSession( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());
    SESession seSession(session_ptr);
    seSession.close();
    ReportSuccess(out);
}


void SecureElementInstance::CloseChannels( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    Session* session_ptr = (Session*) static_cast<long>(args.get("handle").get<double>());
    SESession seSession(session_ptr);
    seSession.closeChannels();
    ReportSuccess(out);
}

void SecureElementInstance::Transmit( const picojson::value& args, picojson::object& out) {
    LoggerD("Enter");
    const double callback_id = args.get("callbackId").get<double>();
    const picojson::array v_command = args.get("command").get<picojson::value::array>();
    ClientChannel* channel_ptr = (ClientChannel*) static_cast<long>(args.get("handle").get<double>());

    auto open = [this, v_command, channel_ptr](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Transmit APDDU command to secure element");
        try {
            SEChannel seChannel(channel_ptr);
            ByteArray transmit_response = seChannel.transmit(v_command);
            picojson::value result = picojson::value(picojson::array());
            picojson::array& arr = result.get<picojson::array>();
            size_t transmit_response_size = transmit_response.size();
            for (size_t i = 0; i < transmit_response_size; i++) {
                arr.push_back(picojson::value(static_cast<double>(transmit_response[i])));
            }
            ReportSuccess( result, response->get<picojson::object>());
        } catch (const ErrorIO& err) {
            LoggerD("Library reported ErrorIO!");
            ReportError(PlatformResult(ErrorCode::IO_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalState& err) {
            LoggerD("Library reported ErrorIllegalState!");
            ReportError(PlatformResult(ErrorCode::INVALID_STATE_ERR), &response->get<picojson::object>());
        } catch (const ErrorIllegalParameter& err) {
            LoggerD("Library reported ErrorIllegalParameter!");
            ReportError(PlatformResult(ErrorCode::INVALID_VALUES_ERR), &response->get<picojson::object>());
        } catch (const ErrorSecurity& err) {
            LoggerD("Library reported ErrorSecurity!");
            ReportError(PlatformResult(ErrorCode::SECURITY_ERR), &response->get<picojson::object>());
        } catch (const ExceptionBase& err) {
            LoggerD("Library reported ExceptionBase!");
            ReportError(PlatformResult(ErrorCode::UNKNOWN_ERR), &response->get<picojson::object>());
        }
    };

    auto get_response = [this, callback_id](const std::shared_ptr<picojson::value>& response) -> void {
        LoggerD("Getting response");
        picojson::object& obj = response->get<picojson::object>();
        obj.insert(std::make_pair("callbackId", picojson::value(callback_id)));
        Instance::PostMessage(this, response->serialize().c_str());
    };

    TaskQueue::GetInstance().Queue<picojson::value>
        ( open, get_response, std::shared_ptr<picojson::value>(new picojson::value(picojson::object())));
}

} // namespace secureelement
} // namespace extension
