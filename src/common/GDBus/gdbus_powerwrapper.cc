#include "common/GDBus/gdbus_powerwrapper.h"
#include "common/logger.h"
#include <sstream>

namespace common{
namespace gdbus{

GDBusWrapperError::GDBusWrapperError() {}

void GDBusWrapperError::ReportError(GError *error) {
  InitError();
  if (error != nullptr) {
    code_ = static_cast<int>(error->code);
    msg_ = std::string(error->message);
    source_ = "glib";
  }
}

void GDBusWrapperError::ReportError(const std::string &msg) {
  InitError();
  msg_ = msg;
  source_ = "user";
}

void GDBusWrapperError::InitError() {
  code_ = 0;
  msg_ = "";
  source_ = "";
}

std::string GDBusWrapperError::Error() {
  std::stringstream result;
  result << "GDBUS REPORT ERROR: ";
  result << " source: ";
  result << source_;
  result << " code: ";
  result << std::to_string(code_);
  result << " message: ";
  result << msg_;
  result << std::endl;
  return result.str();
}

const std::string GDBusPowerWrapper::kDefaultBusName = "org.tizen.system.deviced";
const std::string GDBusPowerWrapper::kDefaultObjectPath =
    "/Org/Tizen/System/DeviceD/Display";

GDBusPowerWrapper::GDBusPowerWrapper(const std::string &bus_name,
                                     const std::string &object_path,
                                     GBusType bus_type,
                                     GDBusProxyFlags bus_proxy_flags)
    : bus_type_(bus_type),
      bus_proxy_flags_(bus_proxy_flags),
      bus_name_(bus_name),
      object_path_(object_path),
      proxy_(nullptr),
      err_(new GDBusWrapperError()) {}

bool GDBusPowerWrapper::Connect() {
  GError *error = nullptr;
  proxy_ = _auto_gen_org_tizen_system_deviced_display_proxy_new_for_bus_sync(
      bus_type_, bus_proxy_flags_, bus_name_.c_str(), object_path_.c_str(),
      NULL, &error);
  if (!proxy_) {
    err_->ReportError(error);
    return false;
  }
  return true;
}

bool GDBusPowerWrapper::CurrentBrightness(int *result) {
  if (!result) {
    err_->ReportError(
        "Null pointer function CurrentBrightness, parameter: result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_current_brightness_sync(
          proxy_, &out_result, NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = out_result;
  return true;
}

bool GDBusPowerWrapper::CustomBrightness(int *result) {
  if (!result) {
    err_->ReportError(
        "Null pointer in function CustomBrightness, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_custom_brightness_sync(
          proxy_, &out_result, NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusPowerWrapper::HoldBrightness(const int brightness, int *result) {
  if (!result) {
    err_->ReportError(
        "Null pointer in function HoldBrightness, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }

  GError *error = nullptr;
  gint out_result;
  gint in_brightness = static_cast<gint>(brightness);
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_hold_brightness_sync(
          proxy_, in_brightness, &out_result, NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusPowerWrapper::LockState(int *result) {
  return LockStateRaw("lcddim", "staycurstate", "NULL", 0, result);
}

bool GDBusPowerWrapper::LockStateRaw(const std::string &state, const std::string &option1,
                             const std::string &option2, const int timeout,
                             int *result) {
  if (!result) {
    err_->ReportError("Null pointer in function LockState, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }

  GError *error = nullptr;
  const gchar *arg_state = static_cast<const gchar *>(state.c_str());
  const gchar *arg_option1 = static_cast<const gchar *>(option1.c_str());
  const gchar *arg_option2 = static_cast<const gchar *>(option2.c_str());
  const gint _timeout = static_cast<gint>(timeout);

  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_lockstate_sync(
          proxy_, arg_state, arg_option1, arg_option2, _timeout, &out_result,
          NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusPowerWrapper::ReleaseBrightness(int *result) {
  if (!result) {
    err_->ReportError(
        "Null pointer in function ReleaseBrightness, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_release_brightness_sync(
          proxy_, &out_result, NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusPowerWrapper::UnlockState(int *result) {
  return UnlockStateRaw("lcddim", "keeptimer", result);
}

bool GDBusPowerWrapper::UnlockStateRaw(const std::string &state,
                               const std::string &option, int *result) {
  if (!result) {
    err_->ReportError("Null pointer in function UnlockState, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }

  GError *error = nullptr;
  gint out_result;
  const gchar *arg_state = static_cast<const gchar *>(state.c_str());
  const gchar *arg_option = static_cast<const gchar *>(option.c_str());
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_unlockstate_sync(
          proxy_, arg_state, arg_option, &out_result, NULL, &error);
  if (!ret) {
    err_->ReportError(error);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

std::string  GDBusPowerWrapper::GetLastError() { return err_->Error(); }

bool GDBusPowerWrapper::IsConnected() {
  if (!proxy_) {
    return Connect();
  }
  return true;
}

GDBusPowerWrapper::~GDBusPowerWrapper() {
  if (proxy_ != nullptr) {
    g_object_unref(proxy_);
    proxy_ = nullptr;
  }
}

} //namespace gdbus
} //namespace common

