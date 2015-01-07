{% for module in modules %}
// {{module.name}}

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

{% set type_map = {'DOMString':'STRING', 'object':'DICTIONARY', 'Date':'PLATFORM_OBJECT', 'boolean':'BOOLEAN', 'byte':'BYTE', 'octet':'OCTET', 'short':'LONG', 'long':'LONG', 'long long': 'LONG_LONG', 'unsigned short':'UNSIGNED_LONG', 'unsigned long long':'UNSIGNED_LONG_LONG', 'float':'DOUBLE', 'double':'DOUBLE'} %}

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

var ExceptionMap = {
    'UnknownError' : tizen.WebAPIException.UNKNOWN_ERR ,
    'TypeMismatchError' : tizen.WebAPIException.TYPE_MISMATCH_ERR ,
    'InvalidValuesError' : tizen.WebAPIException.INVALID_VALUES_ERR ,
    'IOError' : tizen.WebAPIException.IO_ERR ,
    'ServiceNotAvailableError' : tizen.WebAPIException.SERVICE_NOT_AVAILABLE_ERR ,
    'SecurityError' : tizen.WebAPIException.SECURITY_ERR ,
    'NetworkError' : tizen.WebAPIException.NETWORK_ERR ,
    'NotSupportedError' : tizen.WebAPIException.NOT_SUPPORTED_ERR ,
    'NotFoundError' : tizen.WebAPIException.NOT_FOUND_ERR ,
    'InvalidAccessError' : tizen.WebAPIException.INVALID_ACCESS_ERR ,
    'AbortError' : tizen.WebAPIException.ABORT_ERR ,
    'QuotaExceededError' : tizen.WebAPIException.QUOTA_EXCEEDED_ERR ,
}

function callNative(cmd, args) {
    var json = {'cmd':cmd, 'args':args};
    var argjson = JSON.stringify(json);
    var resultString = extension.internal.sendSyncMessage(argjson)
    var result = JSON.parse(resultString);

    if (typeof result !== 'object') {
        throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR);
    }

    if (result['status'] == 'success') {
        if(result['result']) {
            return result['result'];
        }
        return true;
    } else if (result['status'] == 'error') {
        var err = result['error'];
        if(err) {
            if(ExceptionMap[err.name]) {
                throw new tizen.WebAPIException(ExceptionMap[err.name], err.message);
            } else {
                throw new tizen.WebAPIException(tizen.WebAPIException.UNKNOWN_ERR, err.message);
            }
        }
        return false;
    }
}

{% set multicallback = callback|length() > 1 %}

function callNativeWithCallback(cmd, args, callback) {
    if(callback) {
        var id = nextCallbackId();
        args['callbackId'] = id;
        callbacks[id] = callback;
    }

    return callNative(cmd, args);
}
{% endif %}

function SetReadOnlyProperty(obj, n, v){
    Object.defineProperty(obj, n, {value:v, writable: false});
}

{% for enums in module.getTypes('Enum') %}
var {{enums.name}} = {
    {% for e in enums.childs %}
    '{{e}}': '{{e}}'{% if not loop.last %}, {% endif %} 
    {% endfor %}
};
{% endfor %}

{% for iface in module.getTypes('Interface') %}
{% if iface.exported %}
function {{iface.name}}() {
    // constructor of {{iface.name}}
}

{% if iface.inherit %}
{{iface.name}}.prototype = new {{iface.inherit}}();
{{iface.name}}.prototype.constructor = {{iface.name}};
{% endif %}

{% for operation in iface.getTypes('Operation') %}
{{iface.name}}.prototype.{{operation.name}} = function(
        {%- for arg in operation.arguments -%}
            {%- if not arg.optional -%}
                {%- if not loop.first %}, {% endif -%} 
                {{arg.name}}
            {%- endif -%}
        {%- endfor %}) {
    {% if operation.arguments %}
    var args = validator_.validateArgs(arguments, [
        {% for arg in operation.arguments %}
        {'name' : '{{arg.name}}', 'type': types_.
            {%- if arg.isListener -%}
                LISTENER, 'values' : [
                {%- for listener in arg.listenerType.getTypes('Operation') -%}
                    '{{listener.name}}'{% if not loop.last %}, {% endif %}
                {%- endfor -%}
                ]
            {%- elif arg.isEnum -%}
                ENUM, 'values' : [
                {%- for e in arg.enums -%}
                '{{e}}' {%- if not loop.last -%}, {% endif %}
                {%- endfor -%}
                ]
            {%- elif arg.xtype.array > 0 -%}
                ARRAY
            {%- elif arg.xtype.unions or arg.isTypes -%}
                PLATFORM_OBJECT, 'values' : [
                {%- for union in arg.xtype.unions -%}
                    {{union}} {%- if not loop.last -%}, {% endif -%}
                {%- endfor -%}
                ]
            {%- elif arg.xtype.name in type_map -%}
                {{type_map[arg.xtype.name]}}
            {%- else -%}
                DICTIONARY
            {%- endif -%}
            {%- if arg.optional -%}, optional : true{% endif -%}
            {%- if arg.xtype.nullable -%}, nullable : true{% endif -%}
        }{% if not loop.last %}, {% endif %} 
        {% endfor %}
    ]);
    {% endif %}

    {% if operation.arguments %}
    var nativeParam = {
        {% for arg in operation.primitiveArgs %}
            '{{arg.name}}': args.{{arg.name}}{% if not loop.last %},{% endif %}

        {% endfor %}
    };
    {% for arg in operation.arguments %}
    {% if arg.optional %}
    if (args['{{arg.name}}']) {
        nativeParam['{{arg.name}}'] = args.{{arg.name}};
    }
    {% endif %}
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
                if ( /* put some condition and delete true -> */true ) {
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
    } catch(e) {
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
}

{% endfor %}

{% endif %}
{% endfor %}

exports = new {{tizen}}();
{% if window %}
window.{{window}} = new {{window}}();
{% endif %}

{% endfor %}
