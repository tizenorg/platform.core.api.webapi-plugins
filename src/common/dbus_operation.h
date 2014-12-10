// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_DBUS_OPERATION_H_
#define COMMON_DBUS_OPERATION_H_

#include <string>
#include <vector>
#include <set>

#include <dbus/dbus.h>

namespace common {

class DBusOperation;

class DBusOperationArguments {
 public:
  DBusOperationArguments();
  ~DBusOperationArguments();

  void AddArgumentBool(bool val);
  void AddArgumentInt32(int val);
  void AddArgumentUInt32(unsigned int val);
  void AddArgumentUInt64(uint64_t val);
  void AddArgumentString(const std::string& val);

 private:
  enum class ArgType {
    kTypeBool,
    kTypeInt32,
    kTypeUInt32,
    kTypeUInt64,
    kTypeString
  };

  typedef std::pair<ArgType, void*> ArgumentElement;
  typedef std::vector<ArgumentElement> Arguments;

  Arguments arguments_;

  friend class DBusOperation;

  void AppendVariant(DBusMessageIter* bus_msg_iter);
};

class DBusOperationListener {
 public:
  DBusOperationListener();
  virtual ~DBusOperationListener();

  virtual void OnDBusSignal(int value) = 0;
};

class DBusOperation {
 public:
  DBusOperation(const std::string& destination,
          const std::string& path,
          const std::string& interface);
  virtual ~DBusOperation();

  int InvokeSyncGetInt(const std::string& method,
             DBusOperationArguments* args);

  void RegisterSignalListener(const std::string& signal_name,
                DBusOperationListener* listener);
  void UnregisterSignalListener(const std::string& signal_name,
                  DBusOperationListener* listener);

 private:
  std::string destination_;
  std::string path_;
  std::string interface_;

  typedef std::pair<std::string, DBusOperationListener*> SignalListenerPair;
  typedef std::set<SignalListenerPair> SignalListenerSet;
  SignalListenerSet listeners_;

  DBusConnection* connection_;
  std::string rule_;

  void AddDBusSignalFilter();
  void RemoveDBusSignalFilter();

  DBusHandlerResult DBusSignalFilter(DBusConnection* conn,
                     DBusMessage* message);

  static DBusHandlerResult DBusSignalFilterHandler(DBusConnection* conn,
                           DBusMessage* message,
                           void* user_data);

  static std::set<DBusOperation*> s_objects_;
};

} // namespace common

#endif // COMMON_DBUS_OPERATION_H_
