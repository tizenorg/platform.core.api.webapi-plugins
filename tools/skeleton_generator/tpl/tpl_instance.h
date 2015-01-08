// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  {% for iface in moduleObj.getTypes('Interface') %}
    {% if iface.exported %}
      {% for operation in iface.getTypes('Operation') %}
  void {{operation.native_function}}(const picojson::value& args, picojson::object& out);
      {% endfor %}
    {% endif %}
  {% endfor %}
};

} // namespace {{module.lower}}
} // namespace extension

#endif // {{module.upper}}_{{module.upper}}_INSTANCE_H_
