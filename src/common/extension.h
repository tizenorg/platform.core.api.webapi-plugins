// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMMON_EXTENSION_H_
#define COMMON_EXTENSION_H_

// This is a C++ wrapper over Crosswalk Extension C API. It implements once the
// boilerplate for the common case of mapping XW_Extension and XW_Instance to
// objects of their own. The wrapper deals automatically with creating and
// destroying the objects.
//
// Extension object lives during the lifetime of the extension, and when the
// extension process is properly shutdown, it's destructor will be
// called. Instance objects (there can be many) live during the lifetime of a
// script context associated with a frame in the page. These objects serves as
// storage points for extension specific objects, use them for that.

#include <sys/types.h>

#include <string>
#include <map>
#include <functional>

#include "common/platform_exception.h"
#include "common/platform_result.h"
#include "common/XW_Extension.h"
#include "common/XW_Extension_EntryPoints.h"
#include "common/XW_Extension_Permissions.h"
#include "common/XW_Extension_Runtime.h"
#include "common/XW_Extension_SyncMessage.h"

namespace common {

class Instance;
class Extension;

}  // namespace common


// This function should be implemented by each extension and should return
// an appropriate Extension subclass.
common::Extension* CreateExtension();

namespace common {

// implemented in XW_Extension.cc
// can be called only after the extension is fully created
//    (CreateExtension() has been called)
Extension* GetCurrentExtension();

class Extension {
 public:
  Extension();
  virtual ~Extension();

  // These should be called in the subclass constructor.
  void SetExtensionName(const char* name);
  void SetJavaScriptAPI(const char* api);
  void SetExtraJSEntryPoints(const char** entry_points);
  bool RegisterPermissions(const char* perm_table);

  // This API should be called in the message handler of extension
  bool CheckAPIAccessControl(const char* api_name);

  virtual Instance* CreateInstance();

  std::string GetRuntimeVariable(const char* var_name, unsigned len);

 private:
  friend int32_t (::XW_Initialize)(XW_Extension extension,
                                   XW_GetInterface get_interface);

  static int32_t XW_Initialize(XW_Extension extension,
                               XW_GetInterface get_interface,
                               XW_Initialize_Func initialize,
                               XW_CreatedInstanceCallback created_instance,
                               XW_ShutdownCallback shutdown);

  // XW_Extension callbacks.
  static void OnInstanceCreated(XW_Instance xw_instance, Instance* instance); // modified
  static void OnInstanceDestroyed(XW_Instance xw_instance);
  static void HandleMessage(XW_Instance xw_instance, const char* msg);
  static void HandleSyncMessage(XW_Instance xw_instance, const char* msg);

  XW_Extension xw_extension_;

  class Detail;
};

class Instance {
 public:
  Instance();
  virtual ~Instance();

  void PostMessage(const char* msg);
  void SendSyncReply(const char* reply);

  virtual void Initialize() {}
  virtual void HandleMessage(const char* msg) = 0;
  virtual void HandleSyncMessage(const char* msg) {}

  XW_Instance xw_instance() const { return xw_instance_; }

 private:
  friend class Extension;

  XW_Instance xw_instance_;
};

typedef std::function<void(const picojson::value&, picojson::object&)>
    NativeHandler;

class ParsedInstance : public Instance {
 public:
  ParsedInstance();
  virtual ~ParsedInstance();

 protected:
  void RegisterHandler(const std::string& name, const NativeHandler& func);
  void RegisterSyncHandler(const std::string& name, const NativeHandler& func);

  void ReportSuccess(picojson::object& out);
  void ReportSuccess(const picojson::value& result, picojson::object& out);
  void ReportError(picojson::object& out);
  void ReportError(const PlatformException& ex, picojson::object& out);
  void ReportError(const PlatformResult& error, picojson::object* out);

 private:
  void HandleMessage(const char* msg);
  void HandleSyncMessage(const char* msg);

  void HandleMessage(const char* msg, bool is_sync);
  void HandleException(const PlatformException& ex);
  void HandleError(const PlatformResult& error);

  std::map<std::string, NativeHandler> handler_map_;
};

namespace tools {
void ReportSuccess(picojson::object& out);
void ReportSuccess(const picojson::value& result, picojson::object& out);
void ReportError(picojson::object& out);
void ReportError(const PlatformException& ex, picojson::object& out);
void ReportError(const PlatformResult& error, picojson::object* out);

common::PlatformResult CheckAccess(const std::string& privilege);
common::PlatformResult CheckAccess(const std::vector<std::string>& privileges);

#define CHECK_PRIVILEGE_ACCESS(privilege, out) \
do { \
  auto r = common::tools::CheckAccess(privilege); \
  if (!r) { \
    common::tools::ReportError(r, out); \
    return; \
  } \
} while (0)

#define CHECK_BACKWARD_COMPABILITY_PRIVILEGE_ACCESS(current_priv, prev_priv, out) \
do { \
  auto ret = common::tools::CheckAccess(current_priv); \
  auto ret2 = common::tools::CheckAccess(prev_priv); \
\
  if (!ret && ret2) { \
    ret = ret2; \
  } \
\
  if (!ret) { \
    common::tools::ReportError(ret, out); \
    return; \
  } \
} while (0)

}  // namespace tools

}  // namespace common

#endif  // COMMON_EXTENSION_H_
