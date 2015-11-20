#include "gdbuswrapper.h"

GDBusWrapperError::GDBusWrapperError() { _initError(); }

void GDBusWrapperError::reportError(GError *error) {
  _initError();
  if (error != nullptr) {
    code_ = static_cast<int>(error->code);
    msg_ = static_cast<std::string>(error->message);
    header_ = "------ERROR FROM glib -------";
  }
}

void GDBusWrapperError::reportError(std::string msg) {
  _initError();
  msg_ = msg;
  header_ = "------ERROR FROM user --------";
}

void GDBusWrapperError::_initError(void) {
  code_ = 0;
  msg_ = "";
  header_ = "";
}

void GDBusWrapperError::error(void) {
  std::cout << "\n-----------------------------" << std::endl;
  std::cout << "       REPORT ERROR          " << std::endl;
  std::cout << "-----------------------------" << std::endl;
  std::cout << header_ << std::endl;
  std::cout << "code: " << code_ << std::endl;
  std::cout << "message: " << msg_ << std::endl;
  std::cout << "-----------------------------\n" << std::endl;
}

const std::string GDBusWrapper::DEFAULT_BUS_NAME = "org.tizen.system.deviced";
const std::string GDBusWrapper::DEFAULT_OBJECT_PATH =
    "/Org/Tizen/System/DeviceD/Display";

GDBusWrapper::GDBusWrapper() {}

GDBusWrapper::GDBusWrapper(const std::string bus_name,
                           const std::string object_path, GBusType bus_type,
                           GDBusProxyFlags bus_proxy_flags) {
  bus_name_ = bus_name;
  object_path_ = object_path;
  bus_type_ = bus_type;
  bus_proxy_flags_ = bus_proxy_flags;
  proxy_ = nullptr;
  err_ = std::unique_ptr<GDBusWrapperError>(new GDBusWrapperError());
}

bool GDBusWrapper::Connect() {
  GError *error_ = nullptr;
  proxy_ = _auto_gen_org_tizen_system_deviced_display_proxy_new_for_bus_sync(
      bus_type_, bus_proxy_flags_, bus_name_.c_str(), object_path_.c_str(),
      NULL, &error_);
  if (proxy_ == nullptr) {
    err_->reportError(error_);
    return false;
  }
  return true;
}

bool GDBusWrapper::CurrentBrightness(int *result) {
  if (result == nullptr) {
    err_->reportError(
        "Null pointer function CurrentBrightness, parameter: result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error_ = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_current_brightness_sync(
          proxy_, &out_result, NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = out_result;
  return true;
}

bool GDBusWrapper::CustomBrightness(int *result) {
  if (result == nullptr) {
    err_->reportError(
        "Null pointer in function CustomBrightness, parameter result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error_ = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_custom_brightness_sync(
          proxy_, &out_result, NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusWrapper::isValidTimeout(const int timeout) {
  // TODO: add implementation
  (void)timeout;
  return true;
}

bool GDBusWrapper::isValidHoldBrightness(const int brightness) {
  // TODO: add implementation
  (void)brightness;
  return true;
}

bool GDBusWrapper::HoldBrightness(const int brightness, int *result) {
  if (result == nullptr) {
    err_->reportError(
        "Null pointer in function HoldBrightness, parameter result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }
  if (isValidHoldBrightness(brightness) == false) {
    err_->reportError("Validation false, method isValidHoldBrightness");
    return false;
  }
  GError *error_ = nullptr;
  gint out_result;
  gint in_brightness = static_cast<gint>(brightness);
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_hold_brightness_sync(
          proxy_, in_brightness, &out_result, NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusWrapper::LockState(const std::string state, const std::string option1,
                             const std::string option2, const int timeout,
                             int *result) {
  if (result == nullptr) {
    err_->reportError("Null pointer in function LockState, parameter result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }
  if (isValidTimeout(timeout) == false) {
    err_->reportError("Validation false, method isValidTimeout");
    return false;
  }
  GError *error_ = nullptr;
  const gchar *arg_state = static_cast<const gchar *>(state.c_str());
  const gchar *arg_option1 = static_cast<const gchar *>(option1.c_str());
  const gchar *arg_option2 = static_cast<const gchar *>(option2.c_str());
  const gint _timeout = static_cast<gint>(timeout);

  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_lock_state_sync(
          proxy_, arg_state, arg_option1, arg_option2, _timeout, &out_result,
          NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusWrapper::ReleaseBrightness(int *result) {
  if (result == nullptr) {
    err_->reportError(
        "Null pointer in function ReleaseBrightness, parameter result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }
  GError *error_ = nullptr;
  gint out_result;
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_release_brightness_sync(
          proxy_, &out_result, NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

bool GDBusWrapper::UnlockState(const std::string state,
                               const std::string option, int *result) {
  if (result == nullptr) {
    err_->reportError("Null pointer in function UnlockState, parameter result");
    return false;
  }
  if (is_connected() == false) {
    err_->reportError("No connected to bus, try execute method Connect");
    return false;
  }

  GError *error_ = nullptr;
  gint out_result;
  const gchar *arg_state = static_cast<const gchar *>(state.c_str());
  const gchar *arg_option = static_cast<const gchar *>(option.c_str());
  gboolean ret =
      _auto_gen_org_tizen_system_deviced_display_call_unlock_state_sync(
          proxy_, arg_state, arg_option, &out_result, NULL, &error_);
  if (ret == false) {
    err_->reportError(error_);
    return false;
  }
  *result = static_cast<int>(out_result);
  return true;
}

void GDBusWrapper::error(void) { err_->error(); }

bool GDBusWrapper::is_connected() {
  if (proxy_ == nullptr) {
    return Connect();
  }
  return true;
}

GDBusWrapper::~GDBusWrapper() {
  if (proxy_ != nullptr) {
    g_object_unref(proxy_);
  }
}
