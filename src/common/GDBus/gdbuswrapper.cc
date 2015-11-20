#include "common/GDBus/gdbuswrapper.h"
#include "common/logger.h"

namespace common{
namespace gdbus{

GDBusWrapperError::GDBusWrapperError() { InitError(); }

void GDBusWrapperError::ReportError(GError *error) {
  InitError();
  if (error != nullptr) {
    code_ = static_cast<int>(error->code);
    msg_ = std::string(error->message);
    header_ = "------ERROR FROM glib -------";
  }
}

void GDBusWrapperError::ReportError(const std::string &msg) {
  InitError();
  msg_ = msg;
  header_ = "------ERROR FROM user --------";
}

void GDBusWrapperError::InitError(void) {
  code_ = 0;
  msg_ = "";
  header_ = "";
}

void GDBusWrapperError::Error(void) {
  LoggerE("\n-----------------------------");
  LoggerE("       GDBUS REPORT ERROR      ");
  LoggerE("-----------------------------");
  LoggerE("%s\n", header_.c_str());
  LoggerE("code: %d\n", code_);
  LoggerE("message: %s\n", msg_.c_str());
  LoggerE("-----------------------------\n");
}

const std::string GDBusWrapper::kDefaultBusName = "org.tizen.system.deviced";
const std::string GDBusWrapper::kDefaultObjectPath =
    "/Org/Tizen/System/DeviceD/Display";

GDBusWrapper::GDBusWrapper(const std::string &bus_name,
                           const std::string &object_path, GBusType bus_type,
                           GDBusProxyFlags bus_proxy_flags) : bus_name_(bus_name),
                                                            object_path_(object_path),
                                                            bus_type_(bus_type),
                                                            bus_proxy_flags_(bus_proxy_flags),
                                                            proxy_(nullptr),
                                                            err_(new GDBusWrapperError()) {}

bool GDBusWrapper::Connect() {
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

bool GDBusWrapper::CurrentBrightness(int *result) {
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

bool GDBusWrapper::CustomBrightness(int *result) {
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

bool GDBusWrapper::IsValidTimeout(int timeout) const{
  // TODO: add implementation
  (void)timeout;
  return true;
}

bool GDBusWrapper::IsValidHoldBrightness(int brightness) const{
  // TODO: add implementation
  (void)brightness;
  return true;
}

bool GDBusWrapper::HoldBrightness(const int brightness, int *result) {
  if (!result) {
    err_->ReportError(
        "Null pointer in function HoldBrightness, parameter result");
    return false;
  }
  if (!IsConnected()) {
    err_->ReportError("No connected to bus, try execute method Connect");
    return false;
  }
  if (!IsValidHoldBrightness(brightness)) {
    err_->ReportError("Validation false, method IsValidHoldBrightness");
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

bool GDBusWrapper::LockState(const std::string &state, const std::string &option1,
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
  if (!IsValidTimeout(timeout)) {
    err_->ReportError("Validation false, method IsValidTimeout");
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

bool GDBusWrapper::ReleaseBrightness(int *result) {
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

bool GDBusWrapper::UnlockState(const std::string &state,
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

void GDBusWrapper::LogError() { err_->Error(); }

bool GDBusWrapper::IsConnected() {
  if (!proxy_) {
    return Connect();
  }
  return true;
}

GDBusWrapper::~GDBusWrapper() {
  if (proxy_ != nullptr) {
    g_object_unref(proxy_);
  }
}

} //namespace gdbus
} //namespace common

