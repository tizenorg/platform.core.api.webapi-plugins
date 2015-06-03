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

#ifndef SRC_TVWINDOW_TVWINDOW_INSTANCE_H_
#define SRC_TVWINDOW_TVWINDOW_INSTANCE_H_

#include <string>

#include "common/extension.h"
#include "common/picojson.h"

#include "tvwindow/tvwindow_extension.h"

namespace extension {
namespace tvwindow {

class TVWindowInstance : public common::ParsedInstance {
 public:
  explicit TVWindowInstance(TVWindowExtension const& extension);
  virtual ~TVWindowInstance();

 private:
  virtual void HandleMessage(const char* msg);
  virtual void HandleSyncMessage(const char* msg);
};

}  // namespace tvwindow
}  // namespace extension

#endif  // SRC_TVWINDOW_TVWINDOW_INSTANCE_H_
