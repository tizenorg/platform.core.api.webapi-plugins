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

#include "dbus_operation.h"

#include <string>
#include <sstream>
#include <vector>
#include <set>

#include <dbus/dbus.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "logger.h"
#include "platform_exception.h"

#define DBUS_REPLY_TIMEOUT (-1)

namespace common {

DBusOperationArguments::DBusOperationArguments() {
    LoggerD("Entered");
}

DBusOperationArguments::~DBusOperationArguments() {
    LoggerD("Entered");

    for (auto iter = arguments_.begin(); iter != arguments_.end(); ++iter) {
        ArgType type = iter->first;
        void* p_val = iter->second;

        switch (type) {
        case ArgType::kTypeBool:
        case ArgType::kTypeInt32:
            delete static_cast<int32_t *>(p_val);
            break;

        case ArgType::kTypeUInt32:
            delete static_cast<uint32_t *>(p_val);
            break;

        case ArgType::kTypeUInt64:
            delete static_cast<uint64_t *>(p_val);
            break;

        case ArgType::kTypeString:
            delete[] static_cast<char *>(p_val);
            break;

        default:
            LoggerE("invalid type");
            break;
        }
    }

    arguments_.clear();
}

void DBusOperationArguments::AddArgumentBool(bool val) {
    LoggerD("Entered");

    int32_t* p_val = new int32_t;
    *p_val = val;

    arguments_.push_back(ArgumentElement(ArgType::kTypeBool, p_val));
}

void DBusOperationArguments::AddArgumentInt32(int val) {
    LoggerD("Entered");

    int32_t* p_val = new int32_t;
    *p_val = val;

    arguments_.push_back(ArgumentElement(ArgType::kTypeInt32, p_val));
}

void DBusOperationArguments::AddArgumentUInt32(unsigned int val) {
    LoggerD("Entered");

    uint32_t* p_val = new uint32_t;
    *p_val = val;

    arguments_.push_back(ArgumentElement(ArgType::kTypeUInt32, p_val));
}

void DBusOperationArguments::AddArgumentUInt64(uint64_t val) {
    LoggerD("Entered");

    uint64_t* p_val = new uint64_t;
    *p_val = val;

    arguments_.push_back(ArgumentElement(ArgType::kTypeUInt64, p_val));
}

void DBusOperationArguments::AddArgumentString(const std::string& val) {
    LoggerD("Entered");

    const int length = val.length();

    char* p_val = new char[length * 2];
    strncpy(p_val, val.c_str(), length * 2); // TODO: is it safe?

    arguments_.push_back(ArgumentElement(ArgType::kTypeString, p_val));
}

void DBusOperationArguments::AppendVariant(DBusMessageIter* bus_msg_iter) {
    LoggerD("Entered");

    for (auto iter = arguments_.begin(); iter != arguments_.end(); ++iter) {
        ArgType type = iter->first;
        void *p_val = iter->second;

        switch (type) {
        case ArgType::kTypeBool:
            dbus_message_iter_append_basic(bus_msg_iter, DBUS_TYPE_BOOLEAN, p_val);
            break;

        case ArgType::kTypeInt32:
            dbus_message_iter_append_basic(bus_msg_iter, DBUS_TYPE_INT32, p_val);
            break;

        case ArgType::kTypeUInt32:
            dbus_message_iter_append_basic(bus_msg_iter, DBUS_TYPE_UINT32, p_val);
            break;

        case ArgType::kTypeUInt64:
            dbus_message_iter_append_basic(bus_msg_iter, DBUS_TYPE_UINT64, p_val);
            break;

        case ArgType::kTypeString:
            dbus_message_iter_append_basic(bus_msg_iter, DBUS_TYPE_STRING, &p_val);
            break;

        default:
            throw UnknownException("Wrong debug parameter type");
        }
    }
}

DBusOperationListener::DBusOperationListener() {
    LoggerD("Entered");
}

DBusOperationListener::~DBusOperationListener() {
    LoggerD("Entered");
}

std::set<DBusOperation*> DBusOperation::s_objects_;

DBusOperation::DBusOperation(const std::string& destination,
                             const std::string& path,
                             const std::string& interface) :
                             destination_(destination),
                             path_(path),
                             interface_(interface),
                             connection_(nullptr) {
    LoggerD("Created DBusOperator \"" << destination_ << "\" \""
                                      << path_ << "\":\""
                                      << interface_ << "\"");

    s_objects_.insert(this);
}

DBusOperation::~DBusOperation() {
    LoggerD("Entered");

    if (connection_) {
        dbus_connection_close(connection_);
        dbus_connection_unref(connection_);
    }

    const auto iter = s_objects_.find(this);

    if (s_objects_.end() != iter){
        s_objects_.erase(iter);
    } else {
        LoggerE("Object is not existing in the static pool");
    }
}

int DBusOperation::InvokeSyncGetInt(const std::string& method,
                                    DBusOperationArguments* args) {
    LoggerD("Entered");

    if (!connection_) {
        connection_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, nullptr);
    }

    if (!connection_) {
        LoggerE("dbus_bus_get_private error");
        throw UnknownException("Failed to get dbus connection");
    }

    DBusMessage* msg = dbus_message_new_method_call(destination_.c_str(),
                                                    path_.c_str(),
                                                    interface_.c_str(),
                                                    method.c_str());

    if (!msg) {
        LoggerE("dbus_message_new_method_call error");
        throw UnknownException("Failed to create dbus message");
    }

    DBusMessageIter iter;
    dbus_message_iter_init_append(msg, &iter);

    if (nullptr != args) {
        try {
            args->AppendVariant(&iter);
        } catch (const UnknownException& ex) {
            LoggerE("append_variant error");
            dbus_message_unref(msg);
            throw UnknownException("Failed to append dbus variable");
        }
    }

    DBusError err;
    dbus_error_init(&err);
    DBusMessage* reply = dbus_connection_send_with_reply_and_block(connection_,
                                                                   msg,
                                                                   DBUS_REPLY_TIMEOUT,
                                                                   &err);
    dbus_message_unref(msg);

    if (!reply) {
        LoggerE("dbus_connection_send_with_reply_and_block error " << err.name << ": "
                                                                   << err.message);
        dbus_error_free(&err);
        throw UnknownException("Failed to send request via dbus");
    }

    int result = 0;
    dbus_bool_t ret = dbus_message_get_args(reply,
                                            &err,
                                            DBUS_TYPE_INT32,
                                            &result,
                                            DBUS_TYPE_INVALID);
    dbus_message_unref(reply);

    if (!ret) {
        LoggerE("dbus_message_get_args error " << err.name << ": "
                                               << err.message);
        dbus_error_free(&err);
        throw UnknownException("Failed to get reply from dbus");
    }

    return result;
}

void DBusOperation::RegisterSignalListener(const std::string& signal_name,
                                           DBusOperationListener* listener) {
    LoggerD("Entered");

    AddDBusSignalFilter();

    listeners_.insert(std::make_pair(signal_name, listener));
}

void DBusOperation::UnregisterSignalListener(const std::string& signal_name,
                                             DBusOperationListener* listener) {
    LoggerD("Entered");
    LoggerD("unregister signal listener : " << signal_name);

    bool signal_found = false;

    for (auto iter = listeners_.begin(); iter != listeners_.end(); ++iter) {
        if (iter->first == signal_name && iter->second == listener) {
            LoggerD("Found and remove");
            listeners_.erase(iter);

            signal_found = true;
            break;
        }
    }

    if (false == signal_found) {
        LoggerE("Failed to find signal handler");
        throw UnknownException("Failed to find signal handler");
    }

    if (listeners_.size() == 0) {
        RemoveDBusSignalFilter();
    }
}

void DBusOperation::AddDBusSignalFilter() {
    LoggerD("Entered");

    if (!connection_) {
        connection_ = dbus_bus_get_private(DBUS_BUS_SYSTEM, nullptr);
    }

    if (!connection_) {
        LoggerE("dbus_bus_get_private error");
        throw UnknownException("Failed to get dbus connection");
    }

    dbus_connection_setup_with_g_main(connection_, nullptr);

    std::stringstream rule;
    rule << "type='signal',sender='" << destination_
            << "',path='" << path_
            << "',interface='" << interface_ << "'";

    rule_ = rule.str();

    DBusError err;
    dbus_error_init(&err);

    dbus_bus_add_match(connection_, rule_.c_str(), &err);

    if (dbus_error_is_set(&err)) {
        LoggerE("dbus_bus_add_match error " << err.name << ": "
                                            << err.message);
        dbus_error_free(&err);
        throw UnknownException("Failed to set rule for dbus signal");
    }

    if (dbus_connection_add_filter(connection_, DBusSignalFilterHandler, this, nullptr) == FALSE) {
        LoggerE("dbus_connection_add_filter error " << err.name << ": "
                                                    << err.message);
        throw UnknownException("Failed to set handler for dbus signal");
    }
}

void DBusOperation::RemoveDBusSignalFilter() {
    LoggerD("Entered");

    DBusError err;
    dbus_error_init(&err);
    dbus_bus_remove_match(connection_, rule_.c_str(), &err);

    if (dbus_error_is_set(&err)) {
        LoggerE("dbus_bus_remove_match error " << err.name << ": "
                                               << err.message);
        dbus_error_free(&err);
        throw UnknownException("Failed to remove rule for dbus signal");
    }

    dbus_connection_remove_filter(connection_, DBusSignalFilterHandler, this);
}

DBusHandlerResult DBusOperation::DBusSignalFilter(DBusConnection* /* conn */,
                                                  DBusMessage* message) {
    LoggerD("Entered");

    DBusError err;
    dbus_error_init(&err);

    int val = 0;
    if (dbus_message_get_args(message, &err, DBUS_TYPE_INT32, &val, DBUS_TYPE_INVALID) == FALSE) {
        LoggerE("dbus_message_get_args error " << err.name << ": "
                                               << err.message);
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    for (auto iter = listeners_.begin(); iter != listeners_.end(); ++iter) {
        if (dbus_message_is_signal(message, interface_.c_str(), iter->first.c_str())) {
            LoggerD("Found to call : " << iter->first);
            iter->second->OnDBusSignal(val);
        }
    }

    return DBUS_HANDLER_RESULT_HANDLED;
}

DBusHandlerResult DBusOperation::DBusSignalFilterHandler(DBusConnection* conn,
                                                         DBusMessage* message,
                                                         void* user_data) {
    LoggerD("invoked callback from dbus");

    DBusOperation* that = static_cast<DBusOperation *>(user_data);

    if (s_objects_.end() == s_objects_.find(that)) {
        LoggerE("Object does not exist in the static pool");
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    return that->DBusSignalFilter(conn, message);
}

} // namespace common
