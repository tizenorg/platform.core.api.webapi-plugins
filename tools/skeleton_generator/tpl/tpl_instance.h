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

#ifndef {{module.upper}}_{{module.upper}}_INSTANCE_H_
#define {{module.upper}}_{{module.upper}}_INSTANCE_H_

#include "common/extension.h"

namespace extension {
namespace {{module.lower}} {

class {{module.title}}Instance : public common::ParsedInstance {
 public:
  {{module.title}}Instance();
  virtual ~{{module.title}}Instance();

 private:
  {% for func in cmdtable %}
  void {{func}}(const picojson::value& args, picojson::object& out);
  {% endfor %}
};

} // namespace {{module.lower}}
} // namespace extension

#endif // {{module.upper}}_{{module.upper}}_INSTANCE_H_
