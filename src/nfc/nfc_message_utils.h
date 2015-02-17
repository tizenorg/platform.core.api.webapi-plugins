// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef __TIZEN_NFC_NFC_MESSAGE_UTILS_H_
#define __TIZEN_NFC_NFC_MESSAGE_UTILS_H_

#include <vector>
#include <string>
#include <nfc.h>

#include "common/picojson.h"

namespace extension {
namespace nfc {

typedef std::vector<unsigned char> UCharVector;

class NFCMessageUtils
{
 public:
  static void ToNdefRecords(const nfc_ndef_message_h message, picojson::array& array);
  static void ReportNdefMessageFromData(unsigned char* data, unsigned long size,
                                        picojson::object& out);
  static void ReportNDEFMessage(const picojson::value& args, picojson::object& out);
  static nfc_ndef_message_h NDEFMessageToStruct(const picojson::array& records_array,
                                                const int size);
  static void NDEFMessageToByte(const picojson::value& args, picojson::object& out);
  static void ConstructNdefRecordFromRecordHandle(nfc_ndef_record_h record_handle,
                                                  picojson::object& out);
  static void ReportNdefRecordFromMessage(nfc_ndef_message_h message_handle,
                                          const int index, picojson::object& out);
  static void ReportNDEFRecord(const picojson::value& args, picojson::object& out);
  static void ReportNdefRecordTextFromMessage(nfc_ndef_message_h message_handle,
                                              const int index, picojson::object& out);
  static void ReportNDEFRecordText(const picojson::value& args, picojson::object& out);
  static void ReportNdefRecordURIFromMessage(nfc_ndef_message_h message_handle,
                                             const int index, picojson::object& out);
  static void ReportNDEFRecordURI(const picojson::value& args, picojson::object& out);
  static void ReportNdefRecordMediaFromMessage(nfc_ndef_message_h message_handle,
                                               const int index, picojson::object& out);
  static void ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out);
  static void RemoveMessageHandle(nfc_ndef_message_h message_handle);
};

} // nfc
} // extension

#endif // __TIZEN_NFC_NFC_MESSAGE_UTILS_H_
