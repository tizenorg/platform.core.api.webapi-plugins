// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "networkbearerselection/networkbearerselection_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace networkbearerselection {

namespace {
// The privileges that required in NetworkBearerSelection API
const std::string kPrivilegeNetworkBearerSelection = "";

} // namespace

using namespace common;
using namespace extension::networkbearerselection;

NetworkBearerSelectionInstance::NetworkBearerSelectionInstance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&NetworkBearerSelectionInstance::x, this, _1, _2));
  REGISTER_SYNC("NetworkBearerSelection_requestRouteToHost", NetworkBearerSelectionRequestRouteToHost);
  REGISTER_SYNC("NetworkBearerSelection_releaseRouteToHost", NetworkBearerSelectionReleaseRouteToHost);
  #undef REGISTER_SYNC
}

NetworkBearerSelectionInstance::~NetworkBearerSelectionInstance() {
}


enum NetworkBearerSelectionCallbacks {
  NetworkBearerSelectionRequestRouteToHostCallback,
  NetworkBearerSelectionReleaseRouteToHostCallback
};

static void ReplyAsync(NetworkBearerSelectionInstance* instance, NetworkBearerSelectionCallbacks cbfunc,
                       int callbackId, bool isSuccess, picojson::object& param) {
  param["callbackId"] = picojson::value(static_cast<double>(callbackId));
  param["status"] = picojson::value(isSuccess ? "success" : "error");

  // insert result for async callback to param
  switch(cbfunc) {
    case NetworkBearerSelectionRequestRouteToHostCallback: {
      // do something...
      break;
    }
    case NetworkBearerSelectionReleaseRouteToHostCallback: {
      // do something...
      break;
    }
    default: {
      LoggerE("Invalid Callback Type");
      return;
    }
  }

  picojson::value result = picojson::value(param);

  instance->PostMessage(result.serialize().c_str());
}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }


void NetworkBearerSelectionInstance::NetworkBearerSelectionRequestRouteToHost(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "domainName", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& domainName = args.get("domainName").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
void NetworkBearerSelectionInstance::NetworkBearerSelectionReleaseRouteToHost(const picojson::value& args, picojson::object& out) {
  CHECK_EXIST(args, "callbackId", out)
  CHECK_EXIST(args, "domainName", out)

  int callbackId = static_cast<int>(args.get("callbackId").get<double>());
  const std::string& domainName = args.get("domainName").get<std::string>();

  // implement it

  // call ReplyAsync in later (Asynchronously)

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}


#undef CHECK_EXIST

} // namespace networkbearerselection
} // namespace extension
