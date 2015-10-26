// Copyright (c) 2013 Intel Corporation. All rights reserved.
// Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file is compiled into each plugin, hence its size should be minimized.

#include "common/extension.h"
#include "common/assert.h"
#include "common/logger.h"

namespace {

common::Extension* g_extension = nullptr;

} // namespace

namespace common {

Extension* GetCurrentExtension() {
  Assert(g_extension);
  return g_extension;
}

class Extension::Detail {
 private:
  friend int32_t (::XW_Initialize)(XW_Extension extension,
                                   XW_GetInterface get_interface);

  static int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface);

  // XW_Extension callbacks.
  static void OnInstanceCreated(XW_Instance xw_instance);
  static void OnShutdown(XW_Extension);
};

int32_t Extension::Detail::XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  LoggerD("Enter");
  g_extension = CreateExtension();
  if (!g_extension) {
    LoggerE("Can't initialize extension: CreateExtension() returned NULL.");
    return XW_ERROR;
  }
  return XW_OK;
}

void Extension::Detail::OnInstanceCreated(XW_Instance xw_instance) {
  LoggerD("Enter");
  if (!g_extension) {
    return;
  }
  Instance* instance = g_extension->CreateInstance();
  if (!instance) {
    return;
  }
  Extension::OnInstanceCreated(xw_instance, instance);
}

void Extension::Detail::OnShutdown(XW_Extension) {
  LoggerD("Enter");
  delete g_extension;
  g_extension = nullptr;
}

} // namespace common

// Entry point for CrossWalk plugin
extern "C" int32_t XW_Initialize(XW_Extension extension, XW_GetInterface get_interface) {
  return common::Extension::XW_Initialize(extension, get_interface,
                                          common::Extension::Detail::XW_Initialize,
                                          common::Extension::Detail::OnInstanceCreated,
                                          common::Extension::Detail::OnShutdown);
}
