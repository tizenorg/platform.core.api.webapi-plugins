// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_util.h"

#include <nfc.h>

#include "common/logger.h"
#include "common/platform_exception.h"

using namespace common;

namespace extension {
namespace nfc {

UCharVector NFCUtil::toVector(const unsigned char *ch, const int size)
{
  UCharVector vec(ch, ch + size / sizeof(char));
  return vec;
}

void NFCUtil::throwNFCException(const int errorCode, const char* message)
{
  switch(errorCode) {
    case NFC_ERROR_INVALID_PARAMETER:
    case NFC_ERROR_INVALID_NDEF_MESSAGE:
    case NFC_ERROR_INVALID_RECORD_TYPE:
    case NFC_ERROR_NOT_NDEF_FORMAT:
      throw InvalidValuesException(message);
      break;
    case NFC_ERROR_SECURITY_RESTRICTED:
    case NFC_ERROR_PERMISSION_DENIED:
      throw SecurityException(message);
      break;
    case NFC_ERROR_NOT_ACTIVATED:
    case NFC_ERROR_NOT_SUPPORTED:
    case NFC_ERROR_OPERATION_FAILED:
    case NFC_ERROR_DEVICE_BUSY:
    case NFC_ERROR_NO_DEVICE:
    case NFC_ERROR_TIMED_OUT:
    case NFC_ERROR_OUT_OF_MEMORY:
    case NFC_ERROR_NOT_INITIALIZED:
    default:
      throw UnknownException(message);
      break;
  }
}

std::string NFCUtil::getNFCErrorString(const int error_code)
{
  LOGD("Error code : %d",error_code);
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

std::string NFCUtil::getNFCErrorMessage(const int error_code)
{
  LOGD("Error code : %d",error_code);
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
  }
  return "UnknownError";
}

std::string NFCUtil::toStringNFCTag(nfc_tag_type_e tag_type)
{
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

nfc_tag_type_e NFCUtil::toNfcTagString(const std::string& type_string)
{
  if (GENERIC_TARGET == type_string) {
    return NFC_GENERIC_PICC;
  }
  else if (ISO14443_A == type_string) {
    return NFC_ISO14443_A_PICC;
  }
  else if (ISO14443_4A == type_string) {
    return NFC_ISO14443_4A_PICC;
  }
  else if (ISO14443_3A == type_string) {
    return NFC_ISO14443_3A_PICC;
  }
  else if (MIFARE_MINI == type_string) {
    return NFC_MIFARE_MINI_PICC;
  }
  else if (MIFARE_1K == type_string) {
    return NFC_MIFARE_1K_PICC;
  }
  else if (MIFARE_4K == type_string) {
    return NFC_MIFARE_4K_PICC;
  }
  else if (MIFARE_ULTRA == type_string) {
    return NFC_MIFARE_ULTRA_PICC;
  }
  else if (MIFARE_DESFIRE == type_string) {
    return NFC_MIFARE_DESFIRE_PICC;
  }
  else if (ISO14443_B == type_string) {
    return NFC_ISO14443_B_PICC;
  }
  else if (ISO14443_4B == type_string) {
    return NFC_ISO14443_4B_PICC;
  }
  else if (ISO14443_BPRIME == type_string) {
    return NFC_ISO14443_BPRIME_PICC;
  }
  else if (FELICA == type_string) {
    return NFC_FELICA_PICC;
  }
  else if (JEWEL == type_string) {
    return NFC_JEWEL_PICC;
  }
  else if (ISO15693 == type_string) {
    return NFC_ISO15693_PICC;
  }
  else if (UNKNOWN_TARGET == type_string) {
    return NFC_UNKNOWN_TARGET;
  }
  else {
    throw TypeMismatchException("No Match Tag Type");
  }
}

std::string NFCUtil::toStringCardEmulationMode(
    const nfc_se_card_emulation_mode_type_e mode)
{
  switch (mode)
  {
    case NFC_SE_CARD_EMULATION_MODE_OFF:
      return OFF;
    case NFC_SE_CARD_EMULATION_MODE_ON:
      return ALWAYS_ON;
    default:
      LOGE("No Match Card Emulation mode: %x", mode);
      throw TypeMismatchException("No Match Card Emulation mode");
  }
}

nfc_se_card_emulation_mode_type_e NFCUtil::toCardEmulationMode(
    const std::string &mode_string)
{
  if (mode_string == ALWAYS_ON) {
    return NFC_SE_CARD_EMULATION_MODE_ON;
  } else if (mode_string == OFF) {
    return NFC_SE_CARD_EMULATION_MODE_OFF;
  } else {
    LOGE("No Match Card Emulation mode: %s", mode_string.c_str());
    throw TypeMismatchException("No Match Card Emulation mode");
  }
}

std::string NFCUtil::toStringSecureElementType(const nfc_se_type_e type)
{
  switch (type) {
    case NFC_SE_TYPE_ESE:
      return ESE;
    case NFC_SE_TYPE_UICC:
      return UICC;
    default:
      LOGE("No Match Secure Element Type: %x", type);
      throw TypeMismatchException("No Match Secure Element Type");
  }
}

nfc_se_type_e NFCUtil::toSecureElementType(const std::string &type_string)
{
  if (type_string == ESE) {
    return NFC_SE_TYPE_ESE;
  } else if (type_string == UICC) {
    return NFC_SE_TYPE_UICC;
  } else {
    LOGE("No Match Secure Element Type: %s", type_string.c_str());
    throw TypeMismatchException("No Match Secure Element Type");
  }
}

void NFCUtil::setDefaultFilterValues(std::vector<nfc_tag_type_e>& filter)
{
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

} // NFC
} // DeviceApi
