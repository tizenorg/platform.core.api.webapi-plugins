/* global tizen, xwalk, extension */

// Copyright {{year}} Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

{% for module in modules %}

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

{% if module.async %}
var callbackId = 0;
var callbacks = {};

extension.setMessageListener(function(json) {
  var result = JSON.parse(json);
  var callback = callbacks[result['callbackId']];
  callback(result);
});

function nextCallbackId() {
  return callbackId++;
}

function callNative(cmd, args) {
  var json = {'cmd': cmd, 'args': args};
  var argjson = JSON.stringify(json);
  var resultString = extension.internal.sendSyncMessage(argjson);
  var result = JSON.parse(resultString);

  if (typeof result !== 'object') {
    throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
  }

  if (result['status'] === 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] === 'error') {
    var err = result['error'];
    if (err) {
      throw new tizen.WebAPIException(err.name, err.message);
    }
    return false;
  }
}

{% set multicallback = callback|length() > 1 %}

function callNativeWithCallback(cmd, args, callback) {
  if (callback) {
    var id = nextCallbackId();
    args['callbackId'] = id;
    callbacks[id] = callback;
  }

  return callNative(cmd, args);
}
{% endif %}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {'value': v, 'writable': false});
}

{% for enums in module.getTypes('Enum') %}
var {{enums.name}} = {
  {% for e in enums.childs %}
  '{{e}}': '{{e}}'{% if not loop.last %},{% endif %}

  {% endfor %}
};
{% endfor %}

{% for iface in module.getTypes('Interface') %}
{% if iface.exported  or iface.private %}

{% if iface.private %}
// private constructor
{% endif %}
function {{iface.name}}(
    {%-if iface.constructor -%}
    {%- for arg in iface.constructor.arguments -%}
    {{arg.name}}{%- if not loop.last %}, {% endif -%}
    {%- endfor -%}
    {%- endif -%}) {
  // constructor of {{iface.name}}
  {% if iface.constructor %}
  var nativeParam = {
    {% for arg in iface.constructor.primitiveArgs %}
    '{{arg.name}}': args.{{arg.name}}{% if not loop.last %},{% endif %}

    {% endfor %}
  };
  var syncResult = callNative('{{iface.name}}_constructor', nativeParam);
  {% endif %}

  {% for attribute in iface.getTypes('Attribute') %}
    {% if attribute.existIn == 'ctor' %}
    {% set attrValue = attribute.name %}
    {% elif attribute.existIn %}
    {% set attrValue = attribute.existIn %}
    {% else %}
    {% set attrValue = "null" %}
    {% endif %}
    {% if attribute.readonly %}
  SetReadOnlyProperty(this, '{{attribute.name}}', {{attrValue}}); // read only property
    {% else %}
  this.{{attribute.name}} = {{attrValue}};
    {% endif %}
  {% endfor %}
}

{% if iface.inherit %}
{{iface.name}}.prototype = new {{iface.inherit}}();
{{iface.name}}.prototype.constructor = {{iface.name}};
{% endif %}

{% for operation in iface.getTypes('Operation') %}
{{iface.name}}.prototype.{{operation.name}} = function(
    {%- for arg in operation.arguments -%}
      {%- if not loop.first %}, {% endif -%}
      {{arg.name}}
    {%- endfor %}) {
  {% if operation.arguments %}
  var args = validator_.validateArgs(arguments, [
    {% for arg in operation.arguments %}
    {'name': '{{arg.name}}', 'type': types_.{{arg.validation[0]}}
        {%- if arg.validation[1] -%}
        , 'values': {{arg.validation[1]}} 
        {%- endif -%} }
        {%- if not loop.last %},{% endif %}

    {% endfor %}
  ]);
  {% endif %}

  var nativeParam = {
    {% for arg in operation.primitiveArgs if not arg.optional %}
    '{{arg.name}}': args.{{arg.name}}{% if not loop.last %},{% endif %}

    {% endfor %}
  };

  {% for arg in operation.primitiveArgs if arg.optional %}
  if (args['{{arg.name}}']) {
    nativeParam['{{arg.name}}'] = args.{{arg.name}};
  }
  {% endfor %}

  try {
    {% if operation.async %}
    var syncResult = callNativeWithCallback('{{operation.native_cmd}}', nativeParam, function(result) {
      {% for arg in operation.arguments %}
        {% if arg.isListener %}
          {% set cb = callbacks[arg.xtype.name] %}
          {% if cb.callbackType in ['success', 'error']  %}
      if (result.status === '{{cb.callbackType}}') {
            {% if arg.optional %}
        if (args.{{arg.name}}) {
          args.{{arg.name}}.on{{cb.callbackType}}(/* {{cb.callbackType}} argument */);
        }
            {% else %}
        args.{{arg.name}}.on{{cb.callbackType}}(/* {{cb.callbackType}} argument */);
            {% endif %}
      }
          {% else %}
            {% for cbmethod in cb.getTypes('Operation') %}
      if ( /* put some condition and delete true -> */true) {
        args.{{arg.name}}.{{cbmethod.name}}(/* some argument for {{cbmethod.name}} */);
      }
            {% endfor %}
          {% endif %}
        {% endif %}
      {% endfor %}
    });
    {% else %}
    var syncResult = callNative('{{operation.native_cmd}}', nativeParam);
    {% endif %}
    // if you need synchronous result from native function using 'syncResult'.
  } catch (e) {
    throw e;
  }

  {% if operation.returnInternal %}
  var returnObject = new {{operation.returnInternal.name}}();
  return returnObject;
  {% endif %}
};

{% endfor %}

{% endif %}
{% endfor %}

{% for iface in module.getTypes('Interface') %}
    {% if iface.exported == 'Tizen' %}
exports = new {{iface.name}}();
    {% elif iface.exported == 'Window' %}
window.{{iface.name}} = new {{iface.name}}();
    {% elif iface.exported %}
tizen.{{iface.name}} = {{iface.name}};
    {% endif %}
{% endfor %}

{% endfor %}
