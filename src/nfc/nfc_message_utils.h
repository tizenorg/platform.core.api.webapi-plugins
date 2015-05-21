// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#ifndef __TIZEN_NFC_NFC_MESSAGE_UTILS_H_
#define __TIZEN_NFC_NFC_MESSAGE_UTILS_H_

#include <network/nfc.h>

#include <string>
#include <vector>

#include "common/picojson.h"
#include "common/platform_result.h"

namespace extension {
namespace nfc {

typedef std::vector<unsigned char> UCharVector;

class NFCMessageUtils
{
 public:
  static common::PlatformResult ToNdefRecords(const nfc_ndef_message_h message, picojson::array& array);
  static common::PlatformResult ReportNdefMessageFromData(unsigned char* data, unsigned long size,
                                        picojson::object& out);
  static common::PlatformResult ReportNDEFMessage(const picojson::value& args, picojson::object& out);
  static common::PlatformResult NDEFMessageToStruct(const picojson::array& records_array,
                                                const int size,
                                                nfc_ndef_message_h *message);
  static common::PlatformResult NDEFMessageToByte(const picojson::value& args, picojson::object& out);
  static common::PlatformResult ConstructNdefRecordFromRecordHandle(nfc_ndef_record_h record_handle,
                                                  picojson::object& out);
  static common::PlatformResult ReportNdefRecordFromMessage(nfc_ndef_message_h message_handle,
                                          const int index, picojson::object& out);
  static common::PlatformResult ReportNDEFRecord(const picojson::value& args, picojson::object& out);
  static common::PlatformResult ReportNdefRecordTextFromMessage(nfc_ndef_message_h message_handle,
                                              const int index, picojson::object& out);
  static common::PlatformResult ReportNDEFRecordText(const picojson::value& args, picojson::object& out);
  static common::PlatformResult ReportNdefRecordURIFromMessage(nfc_ndef_message_h message_handle,
                                             const int index, picojson::object& out);
  static common::PlatformResult ReportNDEFRecordURI(const picojson::value& args, picojson::object& out);
  static common::PlatformResult ReportNdefRecordMediaFromMessage(nfc_ndef_message_h message_handle,
                                               const int index, picojson::object& out);
  static common::PlatformResult ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out);
  static void RemoveMessageHandle(nfc_ndef_message_h message_handle);
};

}  // nfc
}  // extension

#endif  // __TIZEN_NFC_NFC_MESSAGE_UTILS_H_
