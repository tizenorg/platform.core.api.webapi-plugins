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
