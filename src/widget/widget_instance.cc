/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "widget/widget_instance.h"

namespace extension {
namespace widget {

namespace {
const std::string kPrivilegeWidget = "http://tizen.org/privilege/widget.viewer";

}  // namespace

WidgetInstance::WidgetInstance() {
  ScopeLogger();
  using std::placeholders::_1;
  using std::placeholders::_2;

#define REGISTER_SYNC(c,x) \
  RegisterSyncHandler(c, std::bind(&WidgetInstance::x, this, _1, _2));

#undef REGISTER_SYNC

#define REGISTER_ASYNC(c, x) \
  RegisterHandler(c, std::bind(&WidgetInstance::x, this, _1, _2));

#undef REGISTER_ASYNC
}

WidgetInstance::~WidgetInstance() {
  ScopeLogger();
}

} // namespace widget
} // namespace extension
