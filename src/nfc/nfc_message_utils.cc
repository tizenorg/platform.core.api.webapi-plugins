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
}

static void removeMessageHandle(nfc_ndef_message_h message_handle)
{
    if (message_handle) {
        int result = nfc_ndef_message_destroy(message_handle);
        if (NFC_ERROR_NONE != result) {
            LOGE("Can't destroy NdefMessage: %d, %s", result,
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
            LOGE("Can't destroy NdefMessage: %d, %s", result,
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
        LOGE("Can't get record's tnf: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            removeMessageHandle(message_handle);
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
        LOGE("Can't get record's type: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            removeMessageHandle(message_handle);
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
        LOGE("Can't get record's id: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            removeMessageHandle(message_handle);
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
        LOGE("Can't get record's payload: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        //once record handle must be released - from inherited classes constructors
        //once record handle cannot be released - from base class constructor
        if (NULL == message_handle) {
            removeRecordHandle(handle);
        }
        else {
            removeMessageHandle(message_handle);
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

void NFCMessageUtils::ReportNDEFMessage(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
}

void NFCMessageUtils::ReportNDEFRecord(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
    const picojson::array& raw_data =
            FromJson<picojson::array>(args.get<picojson::object>(), "rawData");
    const int size = static_cast<unsigned char>(args.get("rawDataSize").get<double>());

    std::unique_ptr<unsigned char[]> data(new unsigned char[size]);

    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
    }
    nfc_ndef_message_h message_handle = NULL;

    int result = nfc_ndef_message_create_from_rawdata(&message_handle, data.get(), size);
    if (NFC_ERROR_NONE != result) {
        LOGE("Can't create NdefMessage from data: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Can't create NdefMessage from data");
    }

    int count;
    result = nfc_ndef_message_get_record_count(message_handle, &count);
    if (NFC_ERROR_NONE != result) {
        LOGE("Can't get record count: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        removeMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get record count");
    }

    nfc_ndef_record_h record_handle = NULL;
    result = nfc_ndef_message_get_record(message_handle, 0, &record_handle);
    if (NFC_ERROR_NONE != result) {
        LOGE("Can't get NdefRecord: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        removeMessageHandle(message_handle);
        NFCUtil::throwNFCException(result, "Can't get NdefRecord");
    }

    short _tnf = getTnfFromHandle(record_handle, message_handle);
    UCharVector _type_name = getTypeNameFromHandle(record_handle, message_handle);
    UCharVector _id = getIdFromHandle(record_handle, message_handle);
    UCharVector _payload = getPayloadFromHandle(record_handle, message_handle);

    //constructing json
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

    removeMessageHandle(message_handle);
}

void NFCMessageUtils::ReportNDEFRecordText(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");
    const std::string& text = args.get("text").get<std::string>();
    const std::string& language_code = args.get("languageCode").get<std::string>();
    const std::string& encoding_str = args.get("encoding").get<std::string>();
    nfc_encode_type_e encoding = convertToNfcEncodeUTF(encoding_str);

    nfc_ndef_record_h handle = NULL;

    int result = nfc_ndef_record_create_text(&handle, text.c_str(),
            language_code.c_str(), encoding);
    if (NFC_ERROR_NONE != result) {
        LOGE("Unknown error while getting text record: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result,"Can't create text record");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
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

    removeRecordHandle(handle);
}

void NFCMessageUtils::ReportNDEFRecordURI(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");

    const std::string& uri = args.get("uri").get<std::string>();

    nfc_ndef_record_h handle = NULL;

    int result = nfc_ndef_record_create_uri(&handle, uri.c_str());
    if(NFC_ERROR_NONE != result) {
        LOGE("Unknown error while creating NdefRecordURI: %d, %s", result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Unknown error while creating NdefRecordURI");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
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

    removeRecordHandle(handle);
}

void NFCMessageUtils::ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out){
    LoggerD("Entered");

    const std::string& mime_type = args.get("mimeType").get<std::string>();
    const picojson::array& raw_data =
            FromJson<picojson::array>(args.get<picojson::object>(), "data");
    const int size = static_cast<unsigned char>(args.get("dataSize").get<double>());

    std::unique_ptr<unsigned char[]> data(new unsigned char[size]);
    for (size_t i = 0; i < size; i++) {
        data[i] = static_cast<unsigned char>(raw_data[i].get<double>());
    }

    nfc_ndef_record_h handle = NULL;

    int result = nfc_ndef_record_create_mime(&handle, mime_type.c_str(), data.get(),
            size);
    if (NFC_ERROR_NONE != result) {
        LOGE("Unknown error while getting mimeType: %s - %d: %s",
            mime_type.c_str(), result,
            NFCUtil::getNFCErrorMessage(result).c_str());
        NFCUtil::throwNFCException(result, "Unknown error while getting mimeType");
    }

    short _tnf = getTnfFromHandle(handle);
    UCharVector _type_name = getTypeNameFromHandle(handle);
    UCharVector _id = getIdFromHandle(handle);
    UCharVector _payload = getPayloadFromHandle(handle);

    //constructing json
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

    removeRecordHandle(handle);
}

} // NFC
} // DeviceApi
