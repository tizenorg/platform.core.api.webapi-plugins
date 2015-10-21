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

#ifndef __TIZEN_NFC_NFCUTIL_H_
#define __TIZEN_NFC_NFCUTIL_H_

#include <network/nfc.h>

#include <string>
#include <vector>

#include "common/platform_result.h"

namespace extension {
namespace nfc {

namespace {
const std::string GENERIC_TARGET = "GENERIC_TARGET";
const std::string ISO14443_A = "ISO14443_A";
const std::string ISO14443_4A = "ISO14443_4A";
const std::string ISO14443_3A = "ISO14443_3A";
const std::string MIFARE_MINI = "MIFARE_MINI";
const std::string MIFARE_1K = "MIFARE_1K";
const std::string MIFARE_4K = "MIFARE_4K";
const std::string MIFARE_ULTRA = "MIFARE_ULTRA";
const std::string MIFARE_DESFIRE = "MIFARE_DESFIRE";
const std::string ISO14443_B = "ISO14443_B";
const std::string ISO14443_4B = "ISO14443_4B";
const std::string ISO14443_BPRIME  = "ISO14443_BPRIME";
const std::string FELICA  = "FELICA";
const std::string JEWEL  = "JEWEL";
const std::string ISO15693  = "ISO15693";
const std::string UNKNOWN_TARGET = "UNKNOWN_TARGET";

const std::string ALWAYS_ON = "ALWAYS_ON";
const std::string OFF = "OFF";

const std::string UNKNOWN_ERROR_NAME_STR = "UnknownError";
const std::string INVALID_VALUES_ERROR_NAME_STR = "InvalidValuesError";
const std::string TIMEOUT_ERROR_NAME_STR = "TimeoutError";
const std::string SERVICE_NOT_AVAILABLE_ERROR_NAME_STR = "ServiceNotAvailableError";
const std::string NOT_SUPPORTED_ERROR_NAME_STR = "NotSupportedError";
}

typedef std::vector<unsigned char> UCharVector;

class NFCUtil
{
 public:
  static UCharVector ToVector(const unsigned char* ch, const int size);
  static common::PlatformResult CodeToResult(const int errorCode,
                                             const std::string& message);
  static std::string getNFCErrorString(const int error_code);
  static const std::string getNFCErrorMessage(const int error_code);
  static std::string ToStringNFCTag(const nfc_tag_type_e tag_type);
  static common::PlatformResult ToNfcTagString(const std::string& type_string,
                                               nfc_tag_type_e* tag_type);
  static common::PlatformResult ToStringCardEmulationMode(
      const nfc_se_card_emulation_mode_type_e card_mode, std::string *mode);
  static common::PlatformResult ToCardEmulationMode(
      const std::string& mode_string,
      nfc_se_card_emulation_mode_type_e* mode);
  static common::PlatformResult ToStringSecureElementType(const nfc_se_type_e se_type, std::string *type);
  static common::PlatformResult ToSecureElementType(const std::string& type_string, nfc_se_type_e *type);
  static void setDefaultFilterValues(std::vector<nfc_tag_type_e>& filter);
  static const char* ToStr(nfc_hce_event_type_e event_type);
  static const char* ToStr(nfc_se_type_e se_type);
  static nfc_card_emulation_category_type_e StringToCategory(const std::string& category_type);
  static unsigned char* DoubleArrayToUCharArray(const picojson::array& array_in);
  static UCharVector DoubleArrayToUCharVector(const picojson::array& array_in);
  static picojson::array FromUCharArray(unsigned char* array, unsigned int apdu_len);
};

} // nfc
} // extension

#endif // __TIZEN_NFC_NFCUTIL_H_
