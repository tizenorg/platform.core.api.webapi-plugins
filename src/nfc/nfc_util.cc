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

#include "nfc/nfc_util.h"

#include "common/assert.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "nfc/defs.h"

using namespace common;

namespace extension {
namespace nfc {

UCharVector NFCUtil::ToVector(const unsigned char* ch, const int size)
{
  LoggerD("Entered");
  UCharVector vec(ch, ch + size / sizeof(char));
  return vec;
}

PlatformResult NFCUtil::CodeToResult(const int errorCode,
                                     const std::string& message) {
  LoggerD("Entered");
  switch(errorCode) {
    case NFC_ERROR_INVALID_PARAMETER:
    case NFC_ERROR_INVALID_NDEF_MESSAGE:
    case NFC_ERROR_INVALID_RECORD_TYPE:
    case NFC_ERROR_NOT_NDEF_FORMAT:
      return PlatformResult(ErrorCode::INVALID_VALUES_ERR, message);
    case NFC_ERROR_SECURITY_RESTRICTED:
    case NFC_ERROR_PERMISSION_DENIED:
      return PlatformResult(ErrorCode::SECURITY_ERR, message);
    case NFC_ERROR_NOT_SUPPORTED:
      return PlatformResult(ErrorCode::NOT_SUPPORTED_ERR, message);
    case NFC_ERROR_NOT_ACTIVATED:
    case NFC_ERROR_OPERATION_FAILED:
    case NFC_ERROR_DEVICE_BUSY:
    case NFC_ERROR_NO_DEVICE:
    case NFC_ERROR_TIMED_OUT:
    case NFC_ERROR_OUT_OF_MEMORY:
    case NFC_ERROR_NOT_INITIALIZED:
    default:
      return PlatformResult(ErrorCode::UNKNOWN_ERR, message);
  }
}

std::string NFCUtil::getNFCErrorString(const int error_code)
{
  LoggerD("Error code : %d",error_code);
  switch(error_code) {
    case NFC_ERROR_ALREADY_ACTIVATED:
    case NFC_ERROR_ALREADY_DEACTIVATED:
      return "";
    case NFC_ERROR_INVALID_PARAMETER:
    case NFC_ERROR_INVALID_NDEF_MESSAGE:
    case NFC_ERROR_INVALID_RECORD_TYPE:
    case NFC_ERROR_NOT_NDEF_FORMAT:
      return INVALID_VALUES_ERROR_NAME_STR;
    case NFC_ERROR_NO_DEVICE:
    case NFC_ERROR_OUT_OF_MEMORY:
    case NFC_ERROR_OPERATION_FAILED:
    case NFC_ERROR_DEVICE_BUSY:
      return UNKNOWN_ERROR_NAME_STR;
    case NFC_ERROR_NOT_ACTIVATED:
      return SERVICE_NOT_AVAILABLE_ERROR_NAME_STR;
    case NFC_ERROR_NOT_SUPPORTED:
      return NOT_SUPPORTED_ERROR_NAME_STR;
    case NFC_ERROR_TIMED_OUT:
      return TIMEOUT_ERROR_NAME_STR;
  }
  return UNKNOWN_ERROR_NAME_STR;
}

const std::string NFCUtil::getNFCErrorMessage(const int error_code) {
  LoggerD("Error code : %d", error_code);
  switch(error_code) {
    case NFC_ERROR_ALREADY_ACTIVATED:
    case NFC_ERROR_ALREADY_DEACTIVATED:
      return "";
    case NFC_ERROR_INVALID_PARAMETER:
      return "Invalid Parameter";
    case NFC_ERROR_INVALID_NDEF_MESSAGE:
      return "Invalid NDEF Message";
    case NFC_ERROR_INVALID_RECORD_TYPE:
      return "Invalid Record Type";
    case NFC_ERROR_NO_DEVICE:
      return "No Device";
    case NFC_ERROR_OUT_OF_MEMORY:
      return "Out Of Memory";
    case NFC_ERROR_NOT_SUPPORTED:
      return "NFC Not Supported";
    case NFC_ERROR_OPERATION_FAILED:
      return "Operation Failed";
    case NFC_ERROR_DEVICE_BUSY:
      return "Device Busy";
    case NFC_ERROR_NOT_ACTIVATED:
      return "NFC Not Activated";
    case NFC_ERROR_TIMED_OUT:
      return "Time Out";
    case NFC_ERROR_READ_ONLY_NDEF:
      return "Read Only NDEF";
    case NFC_ERROR_NO_SPACE_ON_NDEF:
      return "No Space On NDEF";
    case NFC_ERROR_NO_NDEF_MESSAGE:
      return "No NDEF Message";
    case NFC_ERROR_NOT_NDEF_FORMAT:
      return "Not NDEF Format";
    case NFC_ERROR_SECURITY_RESTRICTED:
      return "Security Restricted";
    default:
      return "UnknownError";
  }
}

std::string NFCUtil::ToStringNFCTag(nfc_tag_type_e tag_type)
{
  LoggerD("Entered");
  switch (tag_type) {
    case NFC_GENERIC_PICC:
      return GENERIC_TARGET;
    case NFC_ISO14443_A_PICC:
      return ISO14443_A;
    case NFC_ISO14443_4A_PICC:
      return ISO14443_4A;
    case NFC_ISO14443_3A_PICC:
      return ISO14443_3A;
    case NFC_MIFARE_MINI_PICC:
      return MIFARE_MINI;
    case NFC_MIFARE_1K_PICC:
      return MIFARE_1K;
    case NFC_MIFARE_4K_PICC:
      return MIFARE_4K;
    case NFC_MIFARE_ULTRA_PICC:
      return MIFARE_ULTRA;
    case NFC_MIFARE_DESFIRE_PICC:
      return MIFARE_DESFIRE;
    case NFC_ISO14443_B_PICC:
      return ISO14443_B;
    case NFC_ISO14443_4B_PICC:
      return ISO14443_4B;
    case NFC_ISO14443_BPRIME_PICC:
      return ISO14443_BPRIME;
    case NFC_FELICA_PICC:
      return FELICA;
    case NFC_JEWEL_PICC:
      return JEWEL;
    case NFC_ISO15693_PICC:
      return ISO15693;
    case NFC_UNKNOWN_TARGET:
    default:
      return UNKNOWN_TARGET;
  }
}

PlatformResult NFCUtil::ToNfcTagString(const std::string& type_string, nfc_tag_type_e* tag_type)
{
  LoggerD("Entered");
  if (GENERIC_TARGET == type_string) {
    *tag_type = NFC_GENERIC_PICC;
  }
  else if (ISO14443_A == type_string) {
    *tag_type = NFC_ISO14443_A_PICC;
  }
  else if (ISO14443_4A == type_string) {
    *tag_type = NFC_ISO14443_4A_PICC;
  }
  else if (ISO14443_3A == type_string) {
    *tag_type = NFC_ISO14443_3A_PICC;
  }
  else if (MIFARE_MINI == type_string) {
    *tag_type = NFC_MIFARE_MINI_PICC;
  }
  else if (MIFARE_1K == type_string) {
    *tag_type = NFC_MIFARE_1K_PICC;
  }
  else if (MIFARE_4K == type_string) {
    *tag_type = NFC_MIFARE_4K_PICC;
  }
  else if (MIFARE_ULTRA == type_string) {
    *tag_type = NFC_MIFARE_ULTRA_PICC;
  }
  else if (MIFARE_DESFIRE == type_string) {
    *tag_type = NFC_MIFARE_DESFIRE_PICC;
  }
  else if (ISO14443_B == type_string) {
    *tag_type = NFC_ISO14443_B_PICC;
  }
  else if (ISO14443_4B == type_string) {
    *tag_type = NFC_ISO14443_4B_PICC;
  }
  else if (ISO14443_BPRIME == type_string) {
    *tag_type = NFC_ISO14443_BPRIME_PICC;
  }
  else if (FELICA == type_string) {
    *tag_type = NFC_FELICA_PICC;
  }
  else if (JEWEL == type_string) {
    *tag_type = NFC_JEWEL_PICC;
  }
  else if (ISO15693 == type_string) {
    *tag_type = NFC_ISO15693_PICC;
  }
  else if (UNKNOWN_TARGET == type_string) {
    *tag_type = NFC_UNKNOWN_TARGET;
  }
  else {
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "No Match Tag Type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCUtil::ToStringCardEmulationMode(
    const nfc_se_card_emulation_mode_type_e card_mode, std::string* mode)
{
  LoggerD("Entered");
  switch (card_mode)
  {
    case NFC_SE_CARD_EMULATION_MODE_OFF:
      *mode = OFF;
      break;
    case NFC_SE_CARD_EMULATION_MODE_ON:
      *mode = ALWAYS_ON;
      break;
    default:
      LoggerE("No Match Card Emulation mode: %x", card_mode);
      return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "No Match Card Emulation mode");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCUtil::ToCardEmulationMode(
    const std::string& mode_string,
    nfc_se_card_emulation_mode_type_e* mode) {
  LoggerD("Entered");
  if (mode_string == ALWAYS_ON) {
    *mode = NFC_SE_CARD_EMULATION_MODE_ON;
  } else if (mode_string == OFF) {
    *mode = NFC_SE_CARD_EMULATION_MODE_OFF;
  } else {
    LoggerE("No Match Card Emulation mode: %s", mode_string.c_str());
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "No Match Card Emulation mode");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCUtil::ToStringSecureElementType(const nfc_se_type_e se_type,
                                                  std::string* type) {
  LoggerD("Entered");
  switch (se_type) {
    case NFC_SE_TYPE_ESE:
      *type = DATA_NFC_SE_TYPE_ESE;
      break;
    case NFC_SE_TYPE_UICC:
      *type = DATA_NFC_SE_TYPE_UICC;
      break;
    case NFC_SE_TYPE_HCE:
      *type = DATA_NFC_SE_TYPE_HCE;
      break;
    default:
      LoggerE("No Match Secure Element Type: %x", se_type);
      return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "No Match Secure Element Type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCUtil::ToSecureElementType(const std::string& type_string,
                                            nfc_se_type_e* type) {
  LoggerD("Entered");
  if (type_string == DATA_NFC_SE_TYPE_ESE) {
    *type = NFC_SE_TYPE_ESE;
  } else if (type_string == DATA_NFC_SE_TYPE_UICC) {
    *type = NFC_SE_TYPE_UICC;
  } else if (type_string == DATA_NFC_SE_TYPE_HCE) {
    *type = NFC_SE_TYPE_HCE;
  } else {
    LoggerE("No Match Secure Element Type: %s", type_string.c_str());
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "No Match Secure Element Type");
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

void NFCUtil::setDefaultFilterValues(std::vector<nfc_tag_type_e>& filter)
{
  LoggerD("Entered");
  filter.push_back(NFC_GENERIC_PICC);
  filter.push_back(NFC_ISO14443_A_PICC);
  filter.push_back(NFC_ISO14443_3A_PICC);
  filter.push_back(NFC_ISO14443_3A_PICC);
  filter.push_back(NFC_MIFARE_MINI_PICC);
  filter.push_back(NFC_MIFARE_1K_PICC);
  filter.push_back(NFC_MIFARE_4K_PICC);
  filter.push_back(NFC_MIFARE_ULTRA_PICC);
  filter.push_back(NFC_MIFARE_DESFIRE_PICC);
  filter.push_back(NFC_ISO14443_B_PICC);
  filter.push_back(NFC_ISO14443_4B_PICC);
  filter.push_back(NFC_ISO14443_BPRIME_PICC);
  filter.push_back(NFC_FELICA_PICC);
  filter.push_back(NFC_JEWEL_PICC);
  filter.push_back(NFC_ISO15693_PICC);
  filter.push_back(NFC_UNKNOWN_TARGET);
}

// Convertion of enum to HCEEventType(characters sequence).
const char* NFCUtil::ToStr(nfc_hce_event_type_e event_type) {
  LoggerD("Entered");
  switch (event_type) {
    case NFC_HCE_EVENT_DEACTIVATED:
      return "DEACTIVATED";
    case NFC_HCE_EVENT_ACTIVATED:
      return "ACTIVATED";
    case NFC_HCE_EVENT_APDU_RECEIVED:
      return "APDU_RECEIVED";
    default:
      AssertMsg(false, "That event type is incorrect.");
  }
}

// Convertion of enum to SecureElementType(characters sequence).
// Warning! DISABLE and SDCARD are not mentioned at widl spec.
const char* NFCUtil::ToStr(nfc_se_type_e se_type) {
  LoggerD("Entered");
  switch (se_type) {
    case NFC_SE_TYPE_DISABLE:
      return "DISABLE";
    case NFC_SE_TYPE_ESE:
      return "ESE";
    case NFC_SE_TYPE_UICC:
      return "UICC";
    case NFC_SE_TYPE_SDCARD:
      return "SDCARD";
    case NFC_SE_TYPE_HCE:
      return "HCE";
    default:
      AssertMsg(false, "That event type is incorrect.");
  }
}

// Convertion CardEmulationCategoryType(characters sequence) to enum.
nfc_card_emulation_category_type_e NFCUtil::StringToCategory(const std::string& category_type) {
  LoggerD("Entered");
  if (category_type == "PAYMENT")
    return NFC_CARD_EMULATION_CATEGORY_PAYMENT;
  if (category_type == "OTHER")
    return NFC_CARD_EMULATION_CATEGORY_OTHER;
  AssertMsg(false, "That category type is incorrect.");
}

unsigned char* NFCUtil::DoubleArrayToUCharArray(const picojson::array& array_in) {
  LoggerD("Entered");
  unsigned char* result_array = new unsigned char[array_in.size()];
  for(std::size_t i = 0; i < array_in.size(); ++i) {
    result_array[i] = static_cast<unsigned char>(array_in.at(i).get<double>());
  }
  return result_array;
}

UCharVector NFCUtil::DoubleArrayToUCharVector(const picojson::array& array_in) {
  LoggerD("Entered");
  return ToVector(NFCUtil::DoubleArrayToUCharArray(array_in), array_in.size());
}

picojson::array NFCUtil::FromUCharArray(unsigned char* array,
                                        unsigned int apdu_len) {
  LoggerD("Entered");
  picojson::array apdu_array;
  apdu_array.reserve(apdu_len);
  for(int i = 0; i < apdu_len; ++i)
    apdu_array.push_back(picojson::value(static_cast<double>(array[i])));
  return apdu_array;
}

}  // nfc
}  // extension
