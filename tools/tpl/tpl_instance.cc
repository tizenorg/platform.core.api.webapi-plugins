// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "{{module.lower}}/{{module.lower}}_instance.h"

#include <functional>

#include "common/picojson.h"
#include "common/logger.h"
#include "common/platform_exception.h"

namespace extension {
namespace {{module.lower}} {

namespace {
// The privileges that required in {{module.title}} API
const std::string kPrivilege{{module.title}} = "";

} // namespace

using namespace common;
using namespace extension::{{module.lower}};

{{module.title}}Instance::{{module.title}}Instance() {
  using namespace std::placeholders;
  #define REGISTER_SYNC(c,x) \
    RegisterSyncHandler(c, std::bind(&{{module.title}}Instance::x, this, _1, _2));
  {% for func in cmdtable %}
  REGISTER_SYNC("{{cmdtable[func]}}", {{func}});
  {% endfor %}
  #undef REGISTER_SYNC
}

{{module.title}}Instance::~{{module.title}}Instance() {
}

{% set type_map = {'DOMString':'std::string', 'object':'picojson::object', 'boolean':'bool', 'byte':'int', 'octet':'int', 'short':'int', 'long':'double', 'long long': 'double', 'unsigned short':'int', 'unsigned long long':'double', 'float':'double', 'double':'double'} %}
{% set primitives = ['int', 'double', 'bool'] %}

{% if moduleObj.async %}
enum {{module.title}}Callbacks {
  {% for func in cmdtable %}
  {{func}}Callback{% if not loop.last %}, {%+ endif %}

  {% endfor %}
};

static void ReplyAsync({{module.title}}Instance* instance, {{module.title}}Callbacks cbfunc, 
                       int callbackId, bool isSuccess, picojson::object& param) {
  param["callbackId"] = picojson::value(static_cast<double>(callbackId));
  param["status"] = picojson::value(isSuccess ? "success" : "error");
  
  // insert result for async callback to param
  switch(cbfunc) {
  {% for iface in moduleObj.getTypes('Interface') %}
    {% if iface.exported %}
      {% for operation in iface.getTypes('Operation') %}
    case {{operation.native_function}}Callback: {
      // do something...
      break;
    }
      {% endfor %}
    {% endif %}
  {% endfor %}
    default: {
      LoggerE("Invalid Callback Type");
      return;
    }
  }

  picojson::value result = picojson::value(param);
  
  instance->PostMessage(result.serialize().c_str());
}
{% endif %}

#define CHECK_EXIST(args, name, out) \
    if (!args.contains(name)) {\
      ReportError(TypeMismatchException(name" is required argument"), out);\
      return;\
    }

{% for iface in moduleObj.getTypes('Interface') %}
  {% if iface.exported %}
    {% for operation in iface.getTypes('Operation') %}
void {{module.title}}Instance::{{operation.native_function}}(const picojson::value& args, picojson::object& out) {
  {% if operation.async %}
  CHECK_EXIST(args, "callbackId", out)
  {% endif %}
  {% for arg in operation.arguments %}
  {% if not arg.optional and type_map[arg.xtype.name] %}
  CHECK_EXIST(args, "{{arg.name}}", out)
  {% endif %}
  {% endfor %}

  {% if operation.async %}
  int callbackId = args.get("callbackId").get<int>();
  {% endif %}
  {% for arg in operation.arguments %}
  {% set type = type_map[arg.xtype.name] %}
  {% if type %}
  {%+ if type not in primitives %}const {% endif -%}
  {{type}}{% if type == 'std::string' %}&{% endif %} {{arg.name}} = args.get("{{arg.name}}").get<{{type}}>();
  {% endif %}
  {% endfor %}

  // implement it

  {% if operation.async %}
  // call ReplyAsync in later (Asynchronously)
  {% endif %}

  // if success
  // ReportSuccess(out);
  // if error
  // ReportError(out);
}
    {% endfor %}
  {% endif %}
{% endfor %}


#undef CHECK_EXIST

} // namespace {{module.lower}}
} // namespace extension
