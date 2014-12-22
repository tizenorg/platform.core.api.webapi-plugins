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
    static void ReportNDEFMessage(const picojson::value& args, picojson::object& out);
    static void ReportNDEFRecord(const picojson::value& args, picojson::object& out);
    static void ReportNDEFRecordText(const picojson::value& args, picojson::object& out);
    static void ReportNDEFRecordURI(const picojson::value& args, picojson::object& out);
    static void ReportNDEFRecordMedia(const picojson::value& args, picojson::object& out);
};

} // nfc
} // extension

#endif // __TIZEN_NFC_NFC_MESSAGE_UTILS_H_
