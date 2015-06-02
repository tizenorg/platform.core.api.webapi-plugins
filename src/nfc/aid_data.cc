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

#include "nfc/aid_data.h"
#include "common/logger.h"

namespace extension {
namespace nfc {

AIDData::AIDData(nfc_se_type_e se_type, const char* aid, bool read_only)
    : se_type_(se_type),
      aid_(aid),
      read_only_(read_only)
{
  LoggerD("Entered");
}

picojson::value AIDData::toJSON() const {
  LoggerD("Entered");
  picojson::value retval = picojson::value(picojson::object());
  picojson::object& obj = retval.get<picojson::object>();

  obj["aid"] = picojson::value();
  obj["type"] = picojson::value();
  obj["readOnly"] = picojson::value();
  return retval;
}

}  // nfc
}  // extension
