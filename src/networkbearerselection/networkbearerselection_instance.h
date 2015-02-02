// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_
#define NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace networkbearerselection {

class NetworkBearerSelectionInstance : public common::ParsedInstance {
 public:
  NetworkBearerSelectionInstance();
  virtual ~NetworkBearerSelectionInstance();

 private:
  void NetworkBearerSelectionRequestRouteToHost(const picojson::value& args, picojson::object& out);
  void NetworkBearerSelectionReleaseRouteToHost(const picojson::value& args, picojson::object& out);
};

} // namespace networkbearerselection
} // namespace extension

#endif // NETWORKBEARERSELECTION_NETWORKBEARERSELECTION_INSTANCE_H_
