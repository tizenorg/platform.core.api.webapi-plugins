/* global tizen, xwalk, extension */

// Copyright {{year}} Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

{% for module in modules %}

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

{% set type_map = {
  'DOMString':'STRING',
  'object':'DICTIONARY',
  'Date':'PLATFORM_OBJECT',
  'boolean':'BOOLEAN',
  'byte':'BYTE',
  'octet':'OCTET',
  'short':'LONG',
  'long':'LONG',
  'long long': 'LONG_LONG',
  'unsigned short':'UNSIGNED_LONG',
  'unsigned long long':'UNSIGNED_LONG_LONG',
  'float':'DOUBLE',
  'double':'DOUBLE'
} %}

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

  if (result['status'] == 'success') {
    if (result['result']) {
      return result['result'];
    }
    return true;
  } else if (result['status'] == 'error') {
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
{% if iface.exported %}
function {{iface.name}}(
    {%-if iface.constructor -%}
    {%- for arg in iface.constructor.arguments -%}
    {{arg.name}}{%- if not loop.last %}, {% endif -%}
    {%- endfor -%}
    {%- endif -%}) {
  // constructor of {{iface.name}}
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
    {'name': '{{arg.name}}', 'type': types_.
      {%- if arg.functionOnly -%}
        FUNCTION
      {%- elif arg.isListener -%}
        LISTENER, 'values': [
        {%- for listener in arg.listenerType.getTypes('Operation') -%}
          '{{listener.name}}'{% if not loop.last %}, {% endif %}
        {%- endfor -%}
        ]
      {%- elif arg.isEnum -%}
        ENUM, 'values': [
        {%- for e in arg.enums -%}
        '{{e}}' {%- if not loop.last -%}, {% endif %}
        {%- endfor -%}
        ]
      {%- elif arg.xtype.array > 0 -%}
        ARRAY
      {%- elif arg.xtype.unions or arg.isTypes -%}
        PLATFORM_OBJECT, 'values': [
        {%- for union in arg.xtype.unions -%}
          {{union}} {%- if not loop.last -%}, {% endif -%}
        {%- endfor -%}
        ]
      {%- elif arg.xtype.name in type_map -%}
        {{type_map[arg.xtype.name]}}
      {%- else -%}
        DICTIONARY
      {%- endif -%}
      {%- if arg.optional -%}, 'optional': true{% endif -%}
      {%- if arg.xtype.nullable -%}, 'nullable': true{% endif -%}
    }{% if not loop.last %},{% endif %}

    {% endfor %}
  ]);
  {% endif %}

  {% if operation.arguments %}
  var nativeParam = {
    {% for arg in operation.primitiveArgs if not arg.optional %}
    '{{arg.name}}': args.{{arg.name}}{% if not loop.last %},{% endif %}

    {% endfor %}
  };

  {% for arg in operation.primitiveArgs if arg.optional %}
  if (args['{{arg.name}}']){
    nativeParam['{{arg.name}}'] = args.{{arg.name}};
  }
  {% endfor %}
  {% endif %}
  {% set successcbs = [] %}
  {% set errorcbs = [] %}
  try {
    {% if operation.async %}
    var syncResult = callNativeWithCallback('{{operation.native_cmd}}', nativeParam, function(result) {
    {% for arg in operation.arguments %}
    {% if arg.isListener %}
    {% set cb = callbacks[arg.xtype.name] %}
    {% if cb.callbackType in ['success', 'error']  %}
      if (result.status == '{{cb.callbackType}}') {
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
  {% for attribute in operation.returnInternal.getTypes('Attribute') %}
  {% if attribute.readonly %}
  SetReadOnlyProperty(returnObject, '{{attribute.name}}', {% if attribute.name in operation.argnames -%}
        {{attribute.name}}); // read only property
      {%- else -%}
        null); // read only property
      {%- endif %}

  {% else %}
  returnObject.{{attribute.name}} = {% if attribute.name in operation.argnames -%}
        {{attribute.name}};
      {%- else -%}
        null;
      {%- endif %}

  {% endif %}
  {% endfor %}

  return returnObject;
  {% endif %}
};

{% endfor %}

{% endif %}
{% endfor %}

exports = new {{tizen}}();
{% if window %}
window.{{window}} = new {{window}}();
{% endif %}

{% endfor %}
