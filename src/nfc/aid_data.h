// Copyright 2015 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef NFC_AID_DATA_H_
#define NFC_AID_DATA_H_

#include <network/nfc.h>

#include <vector>

#include "common/picojson.h"

namespace extension {
namespace nfc {

class AIDData {
 public:
  AIDData(nfc_se_type_e se_type, const char* aid, bool read_only);
  picojson::value toJSON() const;

 private:
  nfc_se_type_e se_type_;
  const char* aid_;
  bool read_only_;
};

typedef std::vector<AIDData> AIDDataVector;

}  // nfc
}  // extension

#endif  // NFC_AID_DATA_H_