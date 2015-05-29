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

{% for module in modules %}

var utils_ = xwalk.utils;
var type_ = utils_.type;
var converter_ = utils_.converter;
var validator_ = utils_.validator;
var types_ = validator_.Types;
var native_ = new xwalk.utils.NativeManager(extension);

{% if module.async %}
function ListenerManager(native, listenerName, handle) {
  this.listeners = {};
  this.nextId = 1;
  this.nativeSet = false;
  this.native = native;
  this.listenerName = listenerName;
  this.handle = handle || function(msg, listener, watchId) {};
}

ListenerManager.prototype.addListener = function(callback, nativeCall, data) {
  var id = this.nextId;
  if (!this.nativeSet) {
    this.native.addListener(this.listenerName, function(msg) {
      for (var watchId in this.listeners) {
        if (this.listeners.hasOwnProperty(watchId)) {
          this.handle(msg, this.listeners[watchId], watchId);
        }
      }
    }.bind(this));
    var result = this.native.callSync(nativeCall, data || {});
    if (this.native.isFailure(result)) {
      throw this.native.getErrorObject(result);
    }
    this.nativeSet = true;
  }

  this.listeners[id] = callback;
  ++this.nextId;

  return id;
};

ListenerManager.prototype.removeListener = function(watchId, nativeCall) {
  if (this.listeners.hasOwnProperty(watchId)) {
    delete this.listeners[watchId];
  }

  if (this.nativeSet && type_.isEmptyObject(this.listeners)) {
      this.native.callSync(nativeCall);
      this.native.removeListener(this.listenerName);
      this.nativeSet = false;
  }
};

{% set multicallback = callback|length() > 1 %}
{% for iface in module.getTypes('Interface') %}
  {% for operation in iface.getTypes('Operation') if operation.async %}
    {% for arg in operation.arguments if arg.validation[0] == 'LISTENER' %}
var _{{iface.name}}Listener = new ListenerManager(native_, '{{operation.name}}Listener',
  function(msg, listener, watchId) {
      var d = null;

      switch (msg.action) {
      {% for listener in arg.validation[1] %}
      case '{{listener}}':
        d = msg.data;
        break;
      {% endfor %}

      default:
        console.log('Unknown mode: ' + msg.action);
      return;
      }

      listener[msg.action](d);
    });

    {% endfor %}
  {% endfor %}
{% endfor %}
{% endif %}

function SetReadOnlyProperty(obj, n, v) {
  Object.defineProperty(obj, n, {value: v, writable: false});
}

{% for enums in module.getTypes('Enum') %}
var {{enums.name}} = {
  {% for e in enums.childs %}
  {{e}}: '{{e}}'{% if not loop.last %},{% endif %}

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
  validator_.isConstructorCall(this, {{iface.name}});
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
    {name: '{{arg.name}}', type: types_.{{arg.validation[0]}}
        {%- if arg.validation[1] -%}
        {%- if arg.validation[0] == 'PLATFORM_OBJECT' -%}
        , values: tizen.{{arg.validation[1]|first}}
        {%- else -%}
        , values: {{arg.validation[1]}}
        {%- endif -%}
        {%- endif -%}
        {%- if arg.optional -%}, optional: true{% endif -%}
        {%- if arg.xtype.nullable -%}, nullable: true{% endif -%}
    }{%- if not loop.last %},{% endif %}

    {% endfor %}
  ]);
  {% endif %}

  var data = {
    {% for arg in operation.arguments if not arg.validation[0] == 'LISTENER' and not arg.validation[0] == 'FUNCTION' %}
    {{arg.name}}: args.{{arg.name}}{% if not loop.last %},{% endif %}

    {% endfor %}
  };

  {% if operation.async %}
    {% set listenerSet = false %}{% set callbackSet = false %}
    {% for arg in operation.arguments %}
      {% if arg.validation[0] == 'LISTENER' and listenerSet == false %}{% set listenerSet = true %}

  return _{{iface.name}}Listener.addListener(args.{{arg.name}},
      '{{operation.native_cmd}}');

    {%- if arg.name == 'errorCallback' or arg.name == 'callback' -%}
  if (native_.isFailure(result)) {
    native_.callIfPossible(args.{{arg.name}}, native_.getErrorObject(result));
  }
    {% endif %}
  {% endif %}

  {% if arg.validation[0] == 'FUNCTION' and listenerSet == false and callbackSet == false %}{% set callbackSet = true %}
  var callback = function(result) {
  {% for arg in operation.arguments %}
    {% if arg.name == 'errorCallback' or arg.name == 'callback' %}
    if (native_.isFailure(result)) {
      native_.callIfPossible(args.{{arg.name}}, native_.getErrorObject(result));
      return;
    }
    {% endif %}
  {% endfor %}
    native_.callIfPossible(args.{{arg.name}});
  };

  native_.call('{{operation.native_cmd}}', data, callback);
      {% endif %}
    {% endfor %}

  {% else %}

  var result = native_.callSync('{{operation.native_cmd}}', data);

  if (native_.isFailure(result)) {
    throw native_.getErrorObject(result);
  }
  {% endif %}
  {% if operation.returnInternal %}
  return new {{operation.returnInternal.name}}(native_.getResultObject(result));
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
