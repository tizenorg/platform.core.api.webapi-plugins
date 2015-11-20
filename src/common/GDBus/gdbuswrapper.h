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
  void Error(void);

 private:
  std::string header_;
  std::string msg_;
  int code_;
  void InitError(void);
};

class GDBusWrapper {
 public:

  GDBusWrapper(const std::string &bus_name, const std::string &object_path,
               GBusType bus_type = G_BUS_TYPE_SYSTEM,
               GDBusProxyFlags bus_proxy_flags = G_DBUS_PROXY_FLAGS_NONE);

  ~GDBusWrapper();

  static const std::string kDefaultBusName;
  static const std::string kDefaultObjectPath;

  bool Connect();
  bool CurrentBrightness(int *result);
  bool CustomBrightness(int *result);
  bool HoldBrightness(const int brightness, int *result);
  bool LockState(const std::string &state, const std::string &option1,
                 const std::string &option2, const int timeout, int *result);
  bool ReleaseBrightness(int *result);
  bool UnlockState(const std::string &state, const std::string &option,
                   int *result);
  void LogError();

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
};

} //namespace gdbus
} //namespace common

#endif  // GDBUSWRAPPER_H
