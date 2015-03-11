// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc/aid_data.h"

namespace extension {
namespace nfc {

AIDData::AIDData(nfc_se_type_e se_type, const char* aid, bool read_only)
    : se_type_(se_type),
      aid_(aid),
      read_only_(read_only) {}

picojson::value AIDData::toJSON() const {
  picojson::value retval = picojson::value(picojson::object());
  picojson::object& obj = retval.get<picojson::object>();

  obj["aid"] = picojson::value();
  obj["type"] = picojson::value();
  obj["readOnly"] = picojson::value();
  return retval;
}

}  // nfc
}  // extension