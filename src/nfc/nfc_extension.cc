// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "nfc/nfc_extension.h"

#include "nfc/nfc_instance.h"

extern const char kSource_nfc_api[];

common::Extension* CreateExtension() {
  return new NFCExtension;
}

NFCExtension::NFCExtension() {
  SetExtensionName("tizen.nfc");
  SetJavaScriptAPI(kSource_nfc_api);

  const char* entry_points[] = {
      "tizen.NDEFMessage",
      "tizen.NDEFRecord",
      "tizen.NDEFRecordText",
      "tizen.NDEFRecordURI",
      "tizen.NDEFRecordMedia",
      NULL
  };
  SetExtraJSEntryPoints(entry_points);
}

NFCExtension::~NFCExtension() {}

common::Instance* NFCExtension::CreateInstance() {
  return &extension::nfc::NFCInstance::getInstance();;
}
