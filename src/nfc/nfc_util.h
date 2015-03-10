// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef __TIZEN_NFC_NFCUTIL_H_
#define __TIZEN_NFC_NFCUTIL_H_

#include <vector>
#include <string>
#include <nfc.h>

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

const std::string ESE = "ESE";
const std::string UICC = "UICC";

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
  static UCharVector toVector(const unsigned char *ch, const int size);
  static common::PlatformResult CodeToResult(const int errorCode, const char * message);
  static std::string getNFCErrorString(const int error_code);
  static std::string getNFCErrorMessage(const int error_code);
  static std::string ToStringNFCTag(const nfc_tag_type_e tag_type);
  static nfc_tag_type_e toNfcTagString(const std::string& type_string);
  static common::PlatformResult ToStringCardEmulationMode(
      const nfc_se_card_emulation_mode_type_e card_mode, std::string *mode);
  static nfc_se_card_emulation_mode_type_e toCardEmulationMode(
      const std::string& mode_string);
  static common::PlatformResult ToStringSecureElementType(const nfc_se_type_e se_type, std::string *type);
  static common::PlatformResult ToSecureElementType(const std::string& type_string, nfc_se_type_e *type);
  static void setDefaultFilterValues(std::vector<nfc_tag_type_e>& filter);
};

} // nfc
} // extension

#endif // __TIZEN_NFC_NFCUTIL_H_
