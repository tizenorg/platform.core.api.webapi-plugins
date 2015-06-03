/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

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
