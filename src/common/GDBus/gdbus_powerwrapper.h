#ifndef GDBUSWRAPPER_H
#define GDBUSWRAPPER_H

#include <iostream>
#include <string>
#include <memory>
#include "auto_gen_interface.h"

namespace common{
namespace gdbus{

class GDBusWrapperError {
 public:
  GDBusWrapperError();
  void ReportError(GError *error_);
  void ReportError(const std::string &msg);
  std::string Error();

 private:
  std::string source_;
  std::string msg_;
  int code_;
  void InitError();
};

class GDBusPowerWrapper {
 public:
  GDBusPowerWrapper(const std::string &bus_name, const std::string &object_path,
                    GBusType bus_type = G_BUS_TYPE_SYSTEM,
                    GDBusProxyFlags bus_proxy_flags = G_DBUS_PROXY_FLAGS_NONE);

  ~GDBusPowerWrapper();

  static const std::string kDefaultBusName;
  static const std::string kDefaultObjectPath;

  bool Connect();
  bool CurrentBrightness(int *result);
  bool CustomBrightness(int *result);
  bool HoldBrightness(const int brightness, int *result);
  bool LockState(int *result);
  bool ReleaseBrightness(int *result);
  bool UnlockState(int *result);
  std::string GetLastError();

 private:
  GBusType bus_type_;
  GDBusProxyFlags bus_proxy_flags_;

  std::string bus_name_;
  std::string object_path_;
  _auto_genOrgTizenSystemDevicedDisplay *proxy_;
  std::unique_ptr<GDBusWrapperError> err_;

  bool IsConnected();
  bool IsValidHoldBrightness(int brightness) const;
  bool IsValidTimeout(int timeout) const;

  bool LockStateRaw(const std::string &state, const std::string &option1,
                    const std::string &option2, const int timeout, int *result);

  bool UnlockStateRaw(const std::string &state, const std::string &option,
                      int *result);
};

} //namespace gdbus
} //namespace common

#endif  // GDBUSWRAPPER_H
