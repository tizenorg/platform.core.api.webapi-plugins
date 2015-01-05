// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc_message_utils.h"

#include <memory>
#include <nfc.h>

#include "nfc_util.h"

#include "common/logger.h"
#include "common/platform_exception.h"
#include "common/converter.h"

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
void RemoveMessageHandle(nfc_ndef_message_h message_handle)
{
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
    if (record_handle) {
        int result = nfc_ndef_record_destroy(record_handle);
        if (NFC_ERROR_NONE != result) {
            LoggerE("Can't destroy NdefMessage: %d, %s", result,
                NFCUtil::getNFCErrorMessage(result).c_str());
        }
        record_handle = NULL;
    }
}

static short getTnfFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle = NULL)
{
    nfc_record_tnf_e tnf;
    int result = nfc_ndef_record_get_tnf(handle, &tnf);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's tnf: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            RemoveMessageHandle(message_handle);
        }
        NFCUtil::throwNFCException(result, "Can't get record's tnf");
    }

    return static_cast<short>(tnf);
}

static UCharVector getTypeNameFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle = NULL)
{
    unsigned char *type_name;
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
            RemoveMessageHandle(message_handle);
        }
        NFCUtil::throwNFCException(result, "Can't get record's type");
    }
    return NFCUtil::toVector(type_name, type_size);
}

static UCharVector getIdFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle = NULL)
{
    unsigned char *id;
    int id_size, result;

    result = nfc_ndef_record_get_id(handle, &id, &id_size);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's id: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            RemoveMessageHandle(message_handle);
        }
        NFCUtil::throwNFCException(result, "Can't get record's id");
    }

    return NFCUtil::toVector(id, id_size);
}

static UCharVector getPayloadFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle = NULL)
{
    unsigned char *payload;
    unsigned int payload_size;
    int result;

    result = nfc_ndef_record_get_payload(handle, &payload, &payload_size);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's payload: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            RemoveMessageHandle(message_handle);
        }
        NFCUtil::throwNFCException(result, "Can't get record's payload");
    }

    return NFCUtil::toVector(payload, payload_size);
}

static nfc_encode_type_e convertToNfcEncodeUTF(const std::string& encode_string)
{
    if (NFC_TEXT_UTF16 == encode_string) {
        return NFC_ENCODE_UTF_16;
    }
    else {
        return NFC_ENCODE_UTF_8;
    }
}

static std::string convertEncodingToString(nfc_encode_type_e encoding)
{
    if (encoding == NFC_ENCODE_UTF_16) {
        return NFC_TEXT_UTF16;
    } else {
        return NFC_TEXT_UTF8;
    }
}

/* -------------------------------MESSAGE FUNCTIONS------------------------------------ */
void NFCMessageUtils::ToNdefRecords(const nfc_ndef_message_h message, picojson::array& array)
{
    LoggerD("Entered");
    if (NULL != message) {
        int count;
        int result = nfc_ndef_message_get_record_count(message, &count);
        if (NFC_ERROR_NONE != result) {
            LoggerE("Can't get record count: %d, %s", result,
                NFCUtil::getNFCErrorMessage(result).c_str());
            RemoveMessageHandle(message);
            NFCUtil::throwNFCException(result, "Can't get record count");
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
                NFCUtil::throwNFCException(result, "Can't get Ndef Record");
            }
            short tnf = getTnfFromHandle(record_handle, message);
            UCharVector type = getTypeNameFromHandle(record_handle, message);

            if (NFC_RECORD_TNF_MIME_MEDIA == tnf) {
                ReportNdefRecordMediaFromMessage(message, i, record_obj);
                continue;
            } else if (NFC_RECORD_TNF_WELL_KNOWN == tnf) {
                if (!type.empty()) {
                    if (RECORD_TYPE_TEXT == type[0]) {
                        ReportNdefRecordTextFromMessage(message, i, record_obj);
                        continue;
                    }
                    if (RECORD_TYPE_URI == type[0]) {
                        ReportNdefRecordURIFromMessage(message, i, record_obj);
                        continue;
                    }
                }
            }
            ConstructNdefRecordFromRecordHandle(record_handle, record_obj);
        }
    }
}

void NFCMessageUtils::ReportNdefMessageFromData(unsigned char* data, unsigned long size,
        picojson::object& out)
{
    LoggerD("Entered");
    nfc_ndef_message_h message = NULL;

    int result = nfc_ndef_message_create_from_rawdata(&message, data, size);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't create Ndef Message from data");
        NFCUtil::throwNFCException(result, "Can't create Ndef Message from data");
    }
    picojson::value records_array = picojson::value(picojson::array());
    picojson::array& records_array_obj = records_array.get<picojson::array>();
    ToNdefRecords(message, records_array_obj);
    out.insert(std::make_pair("records", records_array));

    RemoveMessageHandle(message);
}

void NFCMessageUtils::ReportNDEFMessage(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
    const picojson::array& raw_data =
            FromJson<picojson::array>(args.get<picojson::object>(), "rawData");
    const int size = static_cast<int>(args.get("rawDataSize").get<double>());

    std::unique_ptr<unsigned char[]> data(new unsigned char[size]);

    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
    }

    ReportNdefMessageFromData(data.get(), size, out);
}

static nfc_ndef_record_h NdefRecordGetHandle(picojson::value& record)
{
    LoggerD("Entered");
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
        throw TypeMismatchException("Type mismatch");
    }

    const int BYTE_ARRAY_MAX = 255;

    nfc_ndef_record_h record_handle = NULL;
    int result = nfc_ndef_record_create(&record_handle,
        tnf, type.get(), type_size > BYTE_ARRAY_MAX ? BYTE_ARRAY_MAX : type_size,
        id.get(), id_size > BYTE_ARRAY_MAX ? BYTE_ARRAY_MAX : id_size,
        payload.get(), payload_size);

    if (NFC_ERROR_NONE != result) {
        removeRecordHandle(record_handle);
        LoggerE("Can't create Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Can't create Ndef Record");
    }
    return record_handle;
}

nfc_ndef_message_h NFCMessageUtils::NDEFMessageToStruct(const picojson::array& records_array,
        const int size)
{
    LoggerD("Entered");
    if (!size) {
        LoggerE("No records in message");
        return NULL;
    }

    nfc_ndef_message_h message = NULL;
    int result = nfc_ndef_message_create(&message);

    if (NFC_ERROR_NONE != result) {
        RemoveMessageHandle(message);
        LoggerE("Can't create Ndef Message: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Can't create Ndef Message");
    }

    for (picojson::value record : records_array) {
        nfc_ndef_record_h record_handle = NdefRecordGetHandle(record);

        result = nfc_ndef_message_append_record(message, record_handle);
        if (NFC_ERROR_NONE != result) {
            LoggerE("record can't be inserted: %d, %s", result,
                NFCUtil::getNFCErrorMessage(result).c_str());
            removeRecordHandle(record_handle);
            RemoveMessageHandle(message);
            NFCUtil::throwNFCException(result, "Invalid NDEF Message");
        }
    }
    return message;
}

void NFCMessageUtils::NDEFMessageToByte(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
    //input
    const picojson::array& records_array =
            FromJson<picojson::array>(args.get<picojson::object>(), "records");
    const int size = static_cast<int>(args.get("recordsSize").get<double>());

    //output
    picojson::value byte_array = picojson::value(picojson::array());
    picojson::array& byte_array_obj = byte_array.get<picojson::array>();

    nfc_ndef_message_h message = NDEFMessageToStruct(records_array, size);
    if (!message) {
        out.insert(std::make_pair("bytes", byte_array));
        return;
    }

    unsigned char *raw_data = NULL;

    unsigned int raw_data_size = 0;
    int result = nfc_ndef_message_get_rawdata(message, &raw_data, &raw_data_size);

    RemoveMessageHandle(message);
    message = NULL;

    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get serial bytes of NDEF message: %d, %s", result,
                NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Can't get serial bytes of NDEF message");
    }

    for (size_t i = 0 ; i < raw_data_size; i++) {
        byte_array_obj.push_back(picojson::value(std::to_string(raw_data[i])));
    }
    if (raw_data) {
        free(raw_data);
    }
    out.insert(std::make_pair("bytes", byte_array));
}

/* -------------------------------RECORD FUNCTIONS------------------------------------ */
static void ConstructRecordJson(short _tnf, const UCharVector& _type_name,
        const UCharVector& _id, const UCharVector& _payload, picojson::object& out)
{
    LoggerD("Entered");
    out.insert(std::make_pair("tnf", std::to_string(_tnf)));

    picojson::value type_array = picojson::value(picojson::array());
    picojson::array& type_array_obj = type_array.get<picojson::array>();
    for (size_t i = 0 ; i < _type_name.size(); i++) {
        type_array_obj.push_back(picojson::value(std::to_string(_type_name[i])));
    }
    out.insert(std::make_pair("type", type_array));

    picojson::value id_array = picojson::value(picojson::array());
    picojson::array& id_array_obj = id_array.get<picojson::array>();
    for (size_t i = 0 ; i < _id.size(); i++) {
        id_array_obj.push_back(picojson::value(std::to_string(_id[i])));
    }
    out.insert(std::make_pair("id", id_array));

    picojson::value payload_array = picojson::value(picojson::array());
    picojson::array& payload_array_obj = payload_array.get<picojson::array>();
    for (size_t i = 0 ; i < _payload.size(); i++) {
        payload_array_obj.push_back(picojson::value(std::to_string(_payload[i])));
    }
    out.insert(std::make_pair("payload", payload_array));
}

void NFCMessageUtils::ConstructNdefRecordFromRecordHandle(nfc_ndef_record_h record_handle,
        picojson::object& out)
{
    LoggerD("Entered");
    short _tnf = getTnfFromHandle(record_handle);
    UCharVector _type_name = getTypeNameFromHandle(record_handle);
    UCharVector _id = getIdFromHandle(record_handle);
    UCharVector _payload = getPayloadFromHandle(record_handle);

    //constructing json
    ConstructRecordJson(_tnf, _type_name, _id, _payload, out);
}

void NFCMessageUtils::ReportNdefRecordFromMessage(nfc_ndef_message_h message_handle,
        const int index, picojson::object& out)
{
    LoggerD("Entered");
    nfc_ndef_record_h record_handle = NULL;
    int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get NdefRecord: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get NdefRecord");
    }

    ConstructNdefRecordFromRecordHandle(record_handle, out);
}

void NFCMessageUtils::ReportNDEFRecord(const picojson::value& args, picojson::object& out){
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
        NFCUtil::throwNFCException(result, "Can't create NdefMessage from data");
    }

    int count;
    result = nfc_ndef_message_get_record_count(message_handle, &count);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record count: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record count");
    }

    ReportNdefRecordFromMessage(message_handle, 0, out);

    RemoveMessageHandle(message_handle);
}

/* -------------------------------RECORD TEXT FUNCTIONS------------------------------------ */
static std::string getTextFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle)
{
    char* text = NULL;
    int result = nfc_ndef_record_get_text(handle, &text);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's text: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record's text");
    }

    std::string text_string(text);
    free(text);
    text = NULL;
    return text_string;
}

static std::string getLanguageCodeFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle)
{
    char* language_code = NULL;
    int result = nfc_ndef_record_get_langcode(handle, &language_code);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's languageCode: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record's languageCode");
    }

    std::string language_string(language_code);
    free(language_code);
    language_code = NULL;
    return language_string;
}

static nfc_encode_type_e getEncodingFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle)
{
    nfc_encode_type_e encoding;
    int result = nfc_ndef_record_get_encode_type(handle, &encoding);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's encoding: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record's encoding");
    }

    return encoding;
}

static void ReportNDEFRecordTextFromText(const std::string& text, const std::string& language_code,
        const std::string& encoding_str, picojson::object& out)
{
    nfc_encode_type_e encoding = convertToNfcEncodeUTF(encoding_str);
    nfc_ndef_record_h handle = NULL;

    int result = nfc_ndef_record_create_text(&handle, text.c_str(),
            language_code.c_str(), encoding);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Unknown error while getting text record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result,"Can't create text record");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
    ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

    removeRecordHandle(handle);
}

void NFCMessageUtils::ReportNdefRecordTextFromMessage(nfc_ndef_message_h message_handle,
        const int index, picojson::object& out)
{
    nfc_ndef_record_h record_handle = NULL;
    //This function just return the pointer of record.
    int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get Ndef Record");
    }

    std::string text = getTextFromHandle(record_handle, message_handle);
    std::string language_code = getLanguageCodeFromHandle(record_handle, message_handle);
    nfc_encode_type_e encoding = getEncodingFromHandle(record_handle, message_handle);
    std::string encoding_str = convertEncodingToString(encoding);

    ReportNDEFRecordTextFromText(text, language_code, encoding_str, out);
    out.insert(std::make_pair("text", text));
    out.insert(std::make_pair("languageCode", language_code));
    out.insert(std::make_pair("encoding", encoding_str));
}

void NFCMessageUtils::ReportNDEFRecordText(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
    const std::string& text = args.get("text").get<std::string>();
    const std::string& language_code = args.get("languageCode").get<std::string>();
    const std::string& encoding_str = args.get("encoding").get<std::string>();

    ReportNDEFRecordTextFromText(text, language_code, encoding_str, out);
}

/* -------------------------------RECORD URI FUNCTIONS------------------------------------ */
static std::string getURIFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle)
{
    char* uri = NULL;
    int result = nfc_ndef_record_get_uri(handle, &uri);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's uri: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record's uri");
    }

    std::string uri_string(uri);
    free(uri);
    uri = NULL;
    return uri_string;
}

static void ReportNDEFRecordURIFromURI(const std::string& uri, picojson::object& out)
{
    nfc_ndef_record_h handle = NULL;

    int result = nfc_ndef_record_create_uri(&handle, uri.c_str());
    if(NFC_ERROR_NONE != result) {
        LoggerE("Unknown error while creating NdefRecordURI: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Unknown error while creating NdefRecordURI");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
    ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

    removeRecordHandle(handle);
}

void NFCMessageUtils::ReportNdefRecordURIFromMessage(nfc_ndef_message_h message_handle,
        const int index, picojson::object& out)
{
    nfc_ndef_record_h record_handle = NULL;
    //This function just return the pointer of record.
    int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get Ndef Record");
    }

    std::string uri = getURIFromHandle(record_handle, message_handle);
    ReportNDEFRecordURIFromURI(uri, out);
    out.insert(std::make_pair("uri", uri));
}

void NFCMessageUtils::ReportNDEFRecordURI(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");

    const std::string& uri = args.get("uri").get<std::string>();
    ReportNDEFRecordURIFromURI(uri, out);
}

/* -------------------------------RECORD MEDIA FUNCTIONS------------------------------------ */
static std::string getMimeTypeFromHandle(nfc_ndef_record_h handle,
        nfc_ndef_message_h message_handle)
{
    char* mime_type = NULL;
    int result = nfc_ndef_record_get_mime_type(handle, &mime_type);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get record's mime_type: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record's mime_type");
    }

    std::string mime_string(mime_type);
    free(mime_type);
    mime_type = NULL;
    return mime_string;
}

void NFCMessageUtils::ReportNdefRecordMediaFromMessage(nfc_ndef_message_h message_handle,
        const int index, picojson::object& out)
{
    nfc_ndef_record_h record_handle = NULL;
    //This function just return the pointer of record.
    int result = nfc_ndef_message_get_record(message_handle, index, &record_handle);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Can't get Ndef Record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        RemoveMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get Ndef Record");
    }

    std::string mime_type = getMimeTypeFromHandle(record_handle, message_handle);
    short _tnf = getTnfFromHandle(record_handle, message_handle);
    UCharVector _type_name = getTypeNameFromHandle(record_handle, message_handle);
    UCharVector _id = getIdFromHandle(record_handle, message_handle);
    UCharVector _payload = getPayloadFromHandle(record_handle, message_handle);
    //constructing json
    ConstructRecordJson(_tnf, _type_name, _id, _payload, out);
    out.insert(std::make_pair("mimeType", mime_type));
}

void NFCMessageUtils::ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out){
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

    int result = nfc_ndef_record_create_mime(&handle, mime_type.c_str(), data.get(),
            size);
    if (NFC_ERROR_NONE != result) {
        LoggerE("Unknown error while getting mimeType: %s - %d: %s",
            mime_type.c_str(), result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Unknown error while getting mimeType");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
    ConstructRecordJson(_tnf, _type_name, _id, _payload, out);

    removeRecordHandle(handle);
}

} // NFC
} // DeviceApi
