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

#include "datacontrol/datacontrol_extension.h"

#include "datacontrol/datacontrol_instance.h"

// This will be generated from datacontrol_api.js
extern const char kSource_datacontrol_api[];

common::Extension* CreateExtension() {
  return new DatacontrolExtension;
}

DatacontrolExtension::DatacontrolExtension() {
  SetExtensionName("tizen.datacontrol");
  SetJavaScriptAPI(kSource_datacontrol_api);
}

DatacontrolExtension::~DatacontrolExtension() {}

common::Instance* DatacontrolExtension::CreateInstance() {
  return new extension::datacontrol::DatacontrolInstance;
}

