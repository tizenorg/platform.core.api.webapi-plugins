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

#include "nfc/nfc_message_utils.h"

#include <memory>

#include "common/converter.h"
#include "common/logger.h"
#include "common/platform_exception.h"
#include "nfc/nfc_util.h"

using namespace common;

namespace extension {
namespace nfc {

namespace {
const char* NFC_TEXT_UTF16 = "UTF16";
const char* NFC_TEXT_UTF8 = "UTF8";

const int RECORD_TYPE_TEXT = 0x54;
const int RECORD_TYPE_URI = 0x55;

const int TNF_MIN = 0;
const int TNF_MAX = 6;

enum nfcTNF{
  NFC_RECORD_TNF_EMPTY = 0,
  NFC_RECORD_TNF_WELL_KNOWN = 1,
  NFC_RECORD_TNF_MIME_MEDIA = 2,
  NFC_RECORD_TNF_URI = 3,
  NFC_RECORD_TNF_EXTERNAL_RTD = 4,
  NFC_RECORD_TNF_UNKNOWN = 5,
  NFC_RECORD_TNF_UNCHANGED = 6
};
}

/* -------------------------------COMMON FUNCTIONS------------------------------------ */
void NFCMessageUtils::RemoveMessageHandle(nfc_ndef_message_h message_handle)
{
  LoggerD("Entered");
  if (message_handle) {
    int result = nfc_ndef_message_destroy(message_handle);
    if (NFC_ERROR_NONE != result) {
      LoggerE("Can't destroy NdefMessage: %d, %s", result,
              NFCUtil::getNFCErrorMessage(result).c_str());
    }
    message_handle = NULL;
  }
}

static void removeRecordHandle(nfc_ndef_record_h record_handle)
{
  LoggerD("Entered");
  if (record_handle) {
    int result = nfc_ndef_record_destroy(record_handle);
    if (NFC_ERROR_NONE != result) {
      LoggerE("Can't destroy NdefMessage: %d, %s", result,
              NFCUtil::getNFCErrorMessage(result).c_str());
    }
    record_handle = NULL;
  }
}

static PlatformResult getTnfFromHandle(nfc_ndef_record_h handle,
                              nfc_ndef_message_h message_handle,
                              short *tnf)
{
  LoggerD("Entered");
  nfc_record_tnf_e record_tnf;
  int result = nfc_ndef_record_get_tnf(handle, &record_tnf);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's tnf: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    //once record handle must be released - from inherited classes constructors
    //once record handle cannot be released - from base class constructor
    if (NULL == message_handle) {
      removeRecordHandle(handle);
    }
    else {
      NFCMessageUtils::RemoveMessageHandle(message_handle);
    }
    return NFCUtil::CodeToResult(result, "Can't get record's tnf");
  }

  *tnf = static_cast<short>(record_tnf);

  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult getTypeNameFromHandle(nfc_ndef_record_h handle,
                                         nfc_ndef_message_h message_handle,
                                         UCharVector *type)
{
  LoggerD("Entered");
  unsigned char* type_name;
  int type_size, result;

  result = nfc_ndef_record_get_type(handle, &type_name, &type_size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's type: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    //once record handle must be released - from inherited classes constructors
    //once record handle cannot be released - from base class constructor
    if (NULL == message_handle) {
      removeRecordHandle(handle);
    }
    else {
      NFCMessageUtils::RemoveMessageHandle(message_handle);
    }
    return NFCUtil::CodeToResult(result, "Can't get record's type");
  }
  *type = NFCUtil::ToVector(type_name, type_size);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult getIdFromHandle(nfc_ndef_record_h handle,
                                   nfc_ndef_message_h message_handle,
                                   UCharVector *id)
{
  LoggerD("Entered");
  unsigned char* tmp_id;
  int id_size, result;

  result = nfc_ndef_record_get_id(handle, &tmp_id, &id_size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's id: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    //once record handle must be released - from inherited classes constructors
    //once record handle cannot be released - from base class constructor
    if (NULL == message_handle) {
      removeRecordHandle(handle);
    }
    else {
      NFCMessageUtils::RemoveMessageHandle(message_handle);
    }
    return NFCUtil::CodeToResult(result, "Can't get record's id");
  }

  *id = NFCUtil::ToVector(tmp_id, id_size);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult getPayloadFromHandle(nfc_ndef_record_h handle,
                                        nfc_ndef_message_h message_handle,
                                        UCharVector *payload)
{
  LoggerD("Entered");
  unsigned char* tmp_payload;
  unsigned int payload_size;
  int result;

  result = nfc_ndef_record_get_payload(handle, &tmp_payload, &payload_size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's payload: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    //once record handle must be released - from inherited classes constructors
    //once record handle cannot be released - from base class constructor
    if (NULL == message_handle) {
      removeRecordHandle(handle);
    }
    else {
      NFCMessageUtils::RemoveMessageHandle(message_handle);
    }
    return NFCUtil::CodeToResult(result, "Can't get record's payload");
  }

  *payload = NFCUtil::ToVector(tmp_payload, payload_size);
  return PlatformResult(ErrorCode::NO_ERROR);
}

static nfc_encode_type_e convertToNfcEncodeUTF(const std::string& encode_string)
{
  LoggerD("Entered");
  if (NFC_TEXT_UTF16 == encode_string) {
    return NFC_ENCODE_UTF_16;
  }
  else {
    return NFC_ENCODE_UTF_8;
  }
}

static std::string convertEncodingToString(nfc_encode_type_e encoding)
{
  LoggerD("Entered");
  if (encoding == NFC_ENCODE_UTF_16) {
    return NFC_TEXT_UTF16;
  } else {
    return NFC_TEXT_UTF8;
  }
}

/* -------------------------------MESSAGE FUNCTIONS------------------------------------ */
PlatformResult NFCMessageUtils::ToNdefRecords(const nfc_ndef_message_h message, picojson::array& array)
{
  LoggerD("Entered");
  if (NULL != message) {
    int count;
    int result = nfc_ndef_message_get_record_count(message, &count);
    if (NFC_ERROR_NONE != result) {
      LoggerE("Can't get record count: %d, %s", result,
              NFCUtil::getNFCErrorMessage(result).c_str());
      RemoveMessageHandle(message);
      return NFCUtil::CodeToResult(result, "Can't get record count");
    }
    for (int i = 0; i < count; ++i) {
      array.push_back(picojson::value(picojson::object()));
      picojson::object& record_obj = array.back().get<picojson::object>();

      nfc_ndef_record_h record_handle = NULL;
      int result = nfc_ndef_message_get_record(message, i, &record_handle);
      if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get Ndef Record: %d, %s", result,
                NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message);
        return NFCUtil::CodeToResult(result, "Can't get Ndef Record");
      }
      short tnf;
      PlatformResult ret = getTnfFromHandle(record_handle, message, &tnf);
      if (ret.IsError()) {
        LoggerE("Error: %s", ret.message().c_str());
        return ret;
      }
      UCharVector type;
      ret = getTypeNameFromHandle(record_handle, message, &type);
      if (ret.IsError()) {
        LoggerE("Error: %s", ret.message().c_str());
        return ret;
      }

      if (NFC_RECORD_TNF_MIME_MEDIA == tnf) {
        ret = ReportNdefRecordMediaFromMessage(message, i, record_obj);
        if (ret.IsError()) {
          LoggerE("Error: %s", ret.message().c_str());
          return ret;
        }
        record_obj.insert(std::make_pair("recordType", picojson::value("RecordMedia")));
        continue;
      } else if (NFC_RECORD_TNF_WELL_KNOWN == tnf) {
        if (!type.empty()) {
          if (RECORD_TYPE_TEXT == type[0]) {
            ret = ReportNdefRecordTextFromMessage(message, i, record_obj);
            if (ret.IsError()) {
              LoggerE("Error: %s", ret.message().c_str());
              return ret;
            }
            record_obj.insert(std::make_pair("recordType", picojson::value("RecordText")));
            continue;
          }
          if (RECORD_TYPE_URI == type[0]) {
            ret = ReportNdefRecordURIFromMessage(message, i, record_obj);
            if (ret.IsError()) {
              LoggerE("Error: %s", ret.message().c_str());
              return ret;
            }
            record_obj.insert(std::make_pair("recordType", picojson::value("RecordURI")));
            continue;
          }
        }
      }
      ret = ConstructNdefRecordFromRecordHandle(record_handle, record_obj);
      if (ret.IsError()) {
        LoggerE("Error: %s", ret.message().c_str());
        return ret;
      }
      record_obj.insert(std::make_pair("recordType", picojson::value("Record")));
    }
  }
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNdefMessageFromData(unsigned char* data, unsigned long size,
                                                picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_message_h message = NULL;

  int result = nfc_ndef_message_create_from_rawdata(&message, data, size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't create Ndef Message from data");
    return NFCUtil::CodeToResult(result, "Can't create Ndef Message from data");
  }
  picojson::value records_array = picojson::value(picojson::array());
  picojson::array& records_array_obj = records_array.get<picojson::array>();
  PlatformResult ret = ToNdefRecords(message, records_array_obj);

  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    RemoveMessageHandle(message);
    return ret;
  }

  out.insert(std::make_pair("records", picojson::value(records_array)));

  RemoveMessageHandle(message);

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNDEFMessage(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");
  const picojson::array& raw_data =
      FromJson<picojson::array>(args.get<picojson::object>(), "rawData");
  const int size = static_cast<int>(args.get("rawDataSize").get<double>());

  std::unique_ptr<unsigned char[]> data(new unsigned char[size]);

  for (size_t i = 0; i < size; i++) {
    data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
  }

  return ReportNdefMessageFromData(data.get(), size, out);
}

static PlatformResult NdefRecordGetHandle(picojson::value& record,
                            nfc_ndef_record_h *record_handle)
{
  LoggerD("Entered");
  if (!record.is<picojson::object>() || !record.contains("tnf") || !record.contains("type") ||
      !record.contains("id") || !record.contains("payload")) {
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Record is empty");
  }
  const picojson::object& record_obj = record.get<picojson::object>();
  short tnf_from_json = static_cast<short>(record.get("tnf").get<double>());
  nfc_record_tnf_e tnf = static_cast<nfc_record_tnf_e>(tnf_from_json);
  const picojson::array& type_data =
      FromJson<picojson::array>(record_obj, "type");
  int type_size = type_data.size();
  std::unique_ptr<unsigned char[]> type(new unsigned char[type_size]);
  for (size_t i = 0; i < type_size; i++) {
    type[i] = static_cast<unsigned char>(type_data[i].get<double>());
  }
  const picojson::array& id_data =
      FromJson<picojson::array>(record_obj, "id");
  int id_size = id_data.size();
  std::unique_ptr<unsigned char[]> id(new unsigned char[id_size]);
  for (size_t i = 0; i < id_size; i++) {
    id[i] = static_cast<unsigned char>(id_data[i].get<double>());
  }
  const picojson::array& payload_data =
      FromJson<picojson::array>(record_obj, "payload");
  int payload_size = payload_data.size();
  std::unique_ptr<unsigned char[]> payload(new unsigned char[payload_size]);
  for (size_t i = 0; i < payload_size; i++) {
    payload[i] = static_cast<unsigned char>(payload_data[i].get<double>());
  }
  if ((tnf_from_json < TNF_MIN) || (tnf_from_json > TNF_MAX)) {
    LoggerE("Not supported tnf");
    return PlatformResult(ErrorCode::TYPE_MISMATCH_ERR, "Type mismatch");
  }
  const int BYTE_ARRAY_MAX = 255;
  nfc_ndef_record_h ndef_record_handle = NULL;
  int result = nfc_ndef_record_create(&ndef_record_handle,
                                      tnf, type.get(), type_size > BYTE_ARRAY_MAX ? BYTE_ARRAY_MAX : type_size,
                                          id.get(), id_size > BYTE_ARRAY_MAX ? BYTE_ARRAY_MAX : id_size,
                                              payload.get(), payload_size);
  if (NFC_ERROR_NONE != result) {
    removeRecordHandle(ndef_record_handle);
    LoggerE("Can't create Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result, "Can't create Ndef Record");
  }
  *record_handle = ndef_record_handle;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::NDEFMessageToStruct(const picojson::array& records_array,
                                                        const int size,
                                                        nfc_ndef_message_h *message)
{
  LoggerD("Entered");
  if (!size) {
    LoggerE("No records in message");
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  nfc_ndef_message_h ndef_message;
  int result = nfc_ndef_message_create(&ndef_message);

  if (NFC_ERROR_NONE != result) {
    RemoveMessageHandle(ndef_message);
    LoggerE("Can't create Ndef Message: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result, "Can't create Ndef Message");
  }

  for (picojson::value record : records_array) {
    nfc_ndef_record_h record_handle;
    PlatformResult ret = NdefRecordGetHandle(record, &record_handle);

    if (ret.IsError()) {
      LoggerE("Error: %s", ret.message().c_str());
      RemoveMessageHandle(ndef_message);
      return ret;
    }

    result = nfc_ndef_message_append_record(ndef_message, record_handle);
    if (NFC_ERROR_NONE != result) {
      LoggerE("record can't be inserted: %d, %s", result,
              NFCUtil::getNFCErrorMessage(result).c_str());
      removeRecordHandle(record_handle);
      RemoveMessageHandle(ndef_message);
      return NFCUtil::CodeToResult(result, "Invalid NDEF Message");
    }
  }
  *message = ndef_message;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::NDEFMessageToByte(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");
  //input
  const picojson::array& records_array =
      FromJson<picojson::array>(args.get<picojson::object>(), "records");
  const int size = static_cast<int>(args.get("recordsSize").get<double>());

  //output
  picojson::value byte_array = picojson::value(picojson::array());
  picojson::array& byte_array_obj = byte_array.get<picojson::array>();

  nfc_ndef_message_h message = NULL;

  PlatformResult ret = NDEFMessageToStruct(records_array, size, &message);

  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  if (!message) {
    out.insert(std::make_pair("bytes", picojson::value(byte_array)));
    return PlatformResult(ErrorCode::NO_ERROR);
  }

  unsigned char* raw_data = NULL;

  unsigned int raw_data_size = 0;
  int result = nfc_ndef_message_get_rawdata(message, &raw_data, &raw_data_size);

  RemoveMessageHandle(message);
  message = NULL;

  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get serial bytes of NDEF message: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result, "Can't get serial bytes of NDEF message");
  }

  for (size_t i = 0 ; i < raw_data_size; i++) {
    byte_array_obj.push_back(picojson::value(std::to_string(raw_data[i])));
  }
  if (raw_data) {
    free(raw_data);
  }
  out.insert(std::make_pair("bytes", picojson::value(byte_array)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

/* -------------------------------RECORD FUNCTIONS------------------------------------ */
static void ConstructRecordJson(short _tnf, const UCharVector& _type_name,
                                const UCharVector& _id, const UCharVector& _payload, picojson::object& out)
{
  LoggerD("Entered");
  out.insert(std::make_pair("tnf", picojson::value(std::to_string(_tnf))));

  picojson::value type_array = picojson::value(picojson::array());
  picojson::array& type_array_obj = type_array.get<picojson::array>();
  for (size_t i = 0 ; i < _type_name.size(); i++) {
    type_array_obj.push_back(picojson::value(std::to_string(_type_name[i])));
  }
  out.insert(std::make_pair("type", picojson::value(type_array)));

  picojson::value id_array = picojson::value(picojson::array());
  picojson::array& id_array_obj = id_array.get<picojson::array>();
  for (size_t i = 0 ; i < _id.size(); i++) {
    id_array_obj.push_back(picojson::value(std::to_string(_id[i])));
  }
  out.insert(std::make_pair("id", picojson::value(id_array)));

  picojson::value payload_array = picojson::value(picojson::array());
  picojson::array& payload_array_obj = payload_array.get<picojson::array>();
  for (size_t i = 0 ; i < _payload.size(); i++) {
    payload_array_obj.push_back(picojson::value(std::to_string(_payload[i])));
  }
  out.insert(std::make_pair("payload", picojson::value(payload_array)));
}

PlatformResult NFCMessageUtils::ConstructNdefRecordFromRecordHandle(nfc_ndef_record_h record_handle,
                                                          picojson::object& out)
{
  LoggerD("Entered");

  short _tnf;
  PlatformResult ret = getTnfFromHandle(record_handle, NULL, &_tnf);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _type_name;
  ret = getTypeNameFromHandle(record_handle, NULL, &_type_name);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _id;
  ret = getIdFromHandle(record_handle, NULL, &_id);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _payload;
  ret = getPayloadFromHandle(record_handle, NULL, &_payload);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  //constructing json
  ConstructRecordJson(_tnf, _type_name, _id, _payload, out);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNdefRecordFromMessage(nfc_ndef_message_h message_handle,
                                                  const int index, picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_record_h record_handle = NULL;
  int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get NdefRecord: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get NdefRecord");
  }

  return ConstructNdefRecordFromRecordHandle(record_handle, out);
}

PlatformResult NFCMessageUtils::ReportNDEFRecord(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");
  const picojson::array& raw_data =
      FromJson<picojson::array>(args.get<picojson::object>(), "rawData");
  const int size = static_cast<int>(args.get("rawDataSize").get<double>());

  std::unique_ptr<unsigned char[]> data(new unsigned char[size]);

  for (size_t i = 0; i < size; i++) {
    data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
  }
  nfc_ndef_message_h message_handle = NULL;

  int result = nfc_ndef_message_create_from_rawdata(&message_handle, data.get(), size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't create NdefMessage from data: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result, "Can't create NdefMessage from data");
  }

  int count;
  result = nfc_ndef_message_get_record_count(message_handle, &count);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record count: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record count");
  }

  PlatformResult ret = ReportNdefRecordFromMessage(message_handle, 0, out);

  RemoveMessageHandle(message_handle);

  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }


  return PlatformResult(ErrorCode::NO_ERROR);
}

/* -------------------------------RECORD TEXT FUNCTIONS------------------------------------ */
static PlatformResult getTextFromHandle(nfc_ndef_record_h handle,
                                     nfc_ndef_message_h message_handle,
                                     std::string *text)
{
  LoggerD("Entered");
  char* tmp_text = NULL;
  int result = nfc_ndef_record_get_text(handle, &tmp_text);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's text: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record's text");
  }

  std::string text_string(tmp_text);
  free(tmp_text);
  tmp_text = NULL;
  *text = text_string;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult getLanguageCodeFromHandle(nfc_ndef_record_h handle,
                                             nfc_ndef_message_h message_handle,
                                             std::string *language)
{
  LoggerD("Entered");
  char* language_code = NULL;
  int result = nfc_ndef_record_get_langcode(handle, &language_code);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's languageCode: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record's languageCode");
  }

  std::string language_string(language_code);
  free(language_code);
  language_code = NULL;
  *language = language_string;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult getEncodingFromHandle(nfc_ndef_record_h handle,
                                               nfc_ndef_message_h message_handle,
                                               nfc_encode_type_e *encoding_type)
{
  LoggerD("Entered");
  nfc_encode_type_e encoding;
  int result = nfc_ndef_record_get_encode_type(handle, &encoding);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's encoding: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record's encoding");
  }

  *encoding_type = encoding;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult ReportNDEFRecordTextFromText(const std::string& text, const std::string& language_code,
                                         const std::string& encoding_str, picojson::object& out)
{
  LoggerD("Entered");
  nfc_encode_type_e encoding = convertToNfcEncodeUTF(encoding_str);
  nfc_ndef_record_h handle = NULL;

  int result = nfc_ndef_record_create_text(&handle, text.c_str(),
                                           language_code.c_str(), encoding);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Unknown error while getting text record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result,"Can't create text record");
  }

  short _tnf;
  PlatformResult ret = getTnfFromHandle(handle, NULL, &_tnf);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _type_name;
  ret = getTypeNameFromHandle(handle, NULL, &_type_name);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _id;
  ret = getIdFromHandle(handle, NULL, &_id);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _payload;
  ret = getPayloadFromHandle(handle, NULL, &_payload);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  //constructing json
  ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

  removeRecordHandle(handle);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNdefRecordTextFromMessage(nfc_ndef_message_h message_handle,
                                                      const int index, picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_record_h record_handle = NULL;
  //This function just return the pointer of record.
  int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get Ndef Record");
  }


  nfc_encode_type_e encoding;
  PlatformResult ret = getEncodingFromHandle(record_handle, message_handle, &encoding);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  std::string encoding_str = convertEncodingToString(encoding);

  std::string text;
  ret = getTextFromHandle(record_handle, message_handle, &text);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  std::string language_code;
  ret = getLanguageCodeFromHandle(record_handle, message_handle, &language_code);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  ret = ReportNDEFRecordTextFromText(text, language_code, encoding_str, out);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  out.insert(std::make_pair("text", picojson::value(text)));
  out.insert(std::make_pair("languageCode", picojson::value(language_code)));
  out.insert(std::make_pair("encoding", picojson::value(encoding_str)));

  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNDEFRecordText(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");
  const std::string& text = args.get("text").get<std::string>();
  const std::string& language_code = args.get("languageCode").get<std::string>();
  const std::string& encoding_str = args.get("encoding").get<std::string>();

  return ReportNDEFRecordTextFromText(text, language_code, encoding_str, out);
}

/* -------------------------------RECORD URI FUNCTIONS------------------------------------ */
static PlatformResult getURIFromHandle(nfc_ndef_record_h handle,
                                    nfc_ndef_message_h message_handle,
                                    std::string* uri_handle)
{
  LoggerD("Entered");
  char* uri = NULL;
  int result = nfc_ndef_record_get_uri(handle, &uri);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's uri: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record's uri");
  }

  std::string uri_string(uri);
  free(uri);
  uri = NULL;
  *uri_handle = uri_string;
  return PlatformResult(ErrorCode::NO_ERROR);
}

static PlatformResult ReportNDEFRecordURIFromURI(const std::string& uri, picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_record_h handle = NULL;

  int result = nfc_ndef_record_create_uri(&handle, uri.c_str());
  if(NFC_ERROR_NONE != result) {
    LoggerE("Unknown error while creating NdefRecordURI: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    return NFCUtil::CodeToResult(result, "Unknown error while creating NdefRecordURI");
  }

  short _tnf;
  PlatformResult ret = getTnfFromHandle(handle, NULL, &_tnf);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _type_name;
  ret = getTypeNameFromHandle(handle, NULL, &_type_name);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _id;
  ret = getIdFromHandle(handle, NULL, &_id);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _payload;
  ret = getPayloadFromHandle(handle, NULL, &_payload);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  //constructing json
  ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

  removeRecordHandle(handle);
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNdefRecordURIFromMessage(nfc_ndef_message_h message_handle,
                                                     const int index, picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_record_h record_handle = NULL;
  //This function just return the pointer of record.
  int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get Ndef Record");
  }

  std::string uri;
  PlatformResult ret = getURIFromHandle(record_handle, message_handle, &uri);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  ret = ReportNDEFRecordURIFromURI(uri, out);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  out.insert(std::make_pair("uri", picojson::value(uri)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNDEFRecordURI(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");

  const std::string& uri = args.get("uri").get<std::string>();
  return ReportNDEFRecordURIFromURI(uri, out);
}

/* -------------------------------RECORD MEDIA FUNCTIONS------------------------------------ */
static PlatformResult getMimeTypeFromHandle(nfc_ndef_record_h handle,
                                         nfc_ndef_message_h message_handle,
                                         std::string *mime)
{
  LoggerD("Entered");
  char* mime_type = NULL;
  int result = nfc_ndef_record_get_mime_type(handle, &mime_type);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get record's mime_type: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    NFCMessageUtils::RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get record's mime_type");
  }

  std::string mime_string(mime_type);
  free(mime_type);
  mime_type = NULL;
  *mime = mime_string;
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNdefRecordMediaFromMessage(nfc_ndef_message_h message_handle,
                                                       const int index, picojson::object& out)
{
  LoggerD("Entered");
  nfc_ndef_record_h record_handle = NULL;
  //This function just return the pointer of record.
  int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    RemoveMessageHandle(message_handle);
    return NFCUtil::CodeToResult(result, "Can't get Ndef Record");
  }

  std::string mime_type;
  PlatformResult ret = getMimeTypeFromHandle(record_handle, message_handle, &mime_type);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  short _tnf;
  ret = getTnfFromHandle(record_handle, message_handle, &_tnf);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _type_name;
  ret = getTypeNameFromHandle(record_handle, message_handle, &_type_name);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _id;
  ret = getIdFromHandle(record_handle, message_handle, &_id);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }
  UCharVector _payload;
  ret = getPayloadFromHandle(record_handle, message_handle, &_payload);
  if (ret.IsError()) {
    LoggerE("Error: %s", ret.message().c_str());
    return ret;
  }

  //constructing json
  ConstructRecordJson(_tnf, _type_name, _id, _payload, out);
  out.insert(std::make_pair("mimeType", picojson::value(mime_type)));
  return PlatformResult(ErrorCode::NO_ERROR);
}

PlatformResult NFCMessageUtils::ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out){
  LoggerD("Entered");

  const std::string& mime_type = args.get("mimeType").get<std::string>();
  const picojson::array& raw_data =
      FromJson<picojson::array>(args.get<picojson::object>(), "data");
  const int size = static_cast<int>(args.get("dataSize").get<double>());

  std::unique_ptr<unsigned char[]> data(new unsigned char[size]);
  for (size_t i = 0; i < size; i++) {
    data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
  }

  nfc_ndef_record_h handle = NULL;

  short _tnf = NFC_RECORD_TNF_UNKNOWN;
  UCharVector _type_name;
  UCharVector _id;
  UCharVector _payload;

  int result = nfc_ndef_record_create_mime(&handle, mime_type.c_str(), data.get(),
                                           size);
  if (NFC_ERROR_NONE != result) {
    LoggerE("Unknown error while getting mimeType: %s - %d: %s",
            mime_type.c_str(), result,
            NFCUtil::getNFCErrorMessage(result).c_str());
    //Do not throw just return default values
    //NFCUtil::throwNFCException(result, "Unknown error while getting mimeType");
  } else {

    PlatformResult ret = getTnfFromHandle(handle, NULL, &_tnf);
    if (ret.IsError()) {
      LoggerE("Error: %s", ret.message().c_str());
      return ret;
    }
    ret = getTypeNameFromHandle(handle, NULL, &_type_name);
    if (ret.IsError()) {
      LoggerE("Error: %s", ret.message().c_str());
      return ret;
    }
    ret = getIdFromHandle(handle, NULL, &_id);
    if (ret.IsError()) {
      LoggerE("Error: %s", ret.message().c_str());
      return ret;
    }
    ret = getPayloadFromHandle(handle, NULL, &_payload);
    if (ret.IsError()) {
      LoggerE("Error: %s", ret.message().c_str());
      return ret;
    }
  }

  //constructing json
  ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

  removeRecordHandle(handle);
  return PlatformResult(ErrorCode::NO_ERROR);
}

} // NFC
} // DeviceApi
