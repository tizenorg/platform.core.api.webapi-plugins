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
 
var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var T_ = xwalk.utils.type;
var bridge = xwalk.utils.NativeBridge(extension, true);

function throwException_(err) {
    throw new WebAPIException(err.code, err.name, err.message);
}

var Property = {
    W: 1 << 0,   // WRITABLE
    E: 1 << 1,   // ENUMERABLE
    C: 1 << 2    // CONFIGURABLE
}

function addTypeToFilter_(data)
{
    var filter = {};

    for(var field in data) {
        filter[field] = data[field];
    }

    if (data instanceof tizen.AttributeFilter) {
        filter.filterType = "AttributeFilter";
        //convert to string
        filter.matchValue = String(filter.matchValue);
    } else if (data instanceof tizen.AttributeRangeFilter) {
        filter.filterType = "AttributeRangeFilter";
    } else if (data instanceof tizen.CompositeFilter) {
        filter.filterType = "CompositeFilter";
        // recursively convert all sub-filters
        filter.filters = [];
        for (var i = 0; i < data.filters.length; ++i) {
            filter.filters[i] = addTypeToFilter_(data.filters[i]);
        }
    } else {
        filter.filterType = "Unknown";
    }

    return filter;
}

/**
 * Example usage:
 * function Messaging () {
 *     propertyFactory_(this, 'ids', [2,3,4], Property.W | Property.E | Property.C);
 *     propertyFactory_(this, 'name', 'Name', Property.E);
 *     propertyFactory_(this, 'age', 25, Property.W);
 *     propertyFactory_(this, 'something', 1);
 *     propertyFactory_(this, 'getSomething', Property.E, {get: function(){return 100;}});
 * }
 * Will produce:
 * var m = new Messaging();
 * {
 *     id: [2,3,4],
 *     name: 'Name',
 *     age: 25
 * }
 *
 * m.name = 'A brand new name';
 * console.log(m.name); // Name
 */
function propertyFactory_(that, name, value, flags, options) {
    flags = flags || 0;
    if (options === null || typeof options !== 'object') {
        options = {};
    }
    if (!(options.get) && !(options.set)) {
        options.value = value;
    }
    if ((flags & Property.W) != 0) { options.writable     = true; }
    if ((flags & Property.E) != 0) { options.enumerable   = true; }
    if ((flags & Property.C) != 0) { options.configurable = true; }
    Object.defineProperty(
        that,
        name,
        options
    );
}

function InternalValues_(data) {
    if (!(this instanceof InternalValues_)) {
        return new InternalValues_(data);
    }
    for(var key in data) {
        if (data.hasOwnProperty(key)) {
            this[key] = data[key];
        }
    }
}

function updateInternal_(internal, data) {
    var values = new InternalValues_(data);
    for(var key in data) {
        if (values.hasOwnProperty(key) && internal.hasOwnProperty(key)) {
            internal[key] = values;
        }
    }
}

/**
 * Specifies the Messaging service tags.
 */
var MessageServiceTag = ['messaging.sms', 'messaging.mms', 'messaging.email'];

function Message(type, data) {
    if (!(this instanceof Message)) {
        throw new TypeError("Constructor called like a function");
    }
    if (MessageServiceTag.indexOf(type) === -1) {
        throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
    }
    if ( !data || typeof data !== 'object') { // 'data' is optional
        data = {};
    }

    // set initial data from internal MessageInit_ object or to default values
    var internal       = data instanceof MessageInit_,
        id             = internal ? data.id             : null,
        conversationId = internal ? data.conversationId : null,
        folderId       = internal ? data.folderId       : null,
        timestamp      = internal ? data.timestamp      : null,
        from           = internal ? data.from           : null,
        hasAttachment  = internal ? data.hasAttachment  : false,
        isRead         = internal ? data.isRead         : false,
        inResponseTo   = internal ? data.inResponseTo   : null;
    // create MessageBody object
    var body = new MessageBody({messageId: id, plainBody: data.plainBody, htmlBody: data.htmlBody});
    // check 'to', 'cc' and 'bcc' fields
    var to = data.to;
    if (!(to instanceof Array)) {
        to = [];
    }
    var cc = data.cc;
    if (!(cc instanceof Array)) {
        cc = [];
    }
    var bcc = data.bcc;
    if (!(bcc instanceof Array)) {
        bcc = [];
    }
    // 'attachments' private variable, getter and setter
    var attachments = (internal ? data.attachments : []) || [];

    var _internal = {
        id: id || null,
        conversationId: conversationId || null,
        folderId: folderId || null,
        type: type,
        timestamp: timestamp || null,
        from: from,
        to: to || [],
        cc: cc || [],
        bcc: bcc || [],
        body: body,
        isRead: isRead || false,
        hasAttachment: hasAttachment || false,
        isHighPriority: data.isHighPriority || false,
        subject: data.subject || '',
        inResponseTo: inResponseTo || null,
        attachments: attachments
    };
    // id
    Object.defineProperty(
        this,
        'id',
        {
            get: function () {return _internal.id;},
            set: function (value) { if (value instanceof InternalValues_) _internal.id = value.id;},
            enumerable: true
        }
    );

    //conversationId
    Object.defineProperty(
        this,
        'conversationId',
        {
            get: function () {return _internal.conversationId;},
            set: function (value) {
                if (value instanceof InternalValues_)
                    _internal.conversationId = value.conversationId;
            },
            enumerable: true
        }
    );

    // folderId
    Object.defineProperty(
        this,
        'folderId',
        {
            get: function () {return _internal.folderId;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.folderId = value.folderId;
            },
            enumerable: true
        }
    );

    // type
    Object.defineProperty(
        this,
        'type',
        {
            get: function () {return _internal.type;},
            set: function (value) {return;},
            enumerable: true
        }
    );

    // timestamp
    Object.defineProperty(
        this,
        'timestamp',
        {
            get: function () {
                return _internal.timestamp ? new Date(_internal.timestamp * 1000) : _internal.timestamp;
            },
            set: function (value) {
                if (value instanceof InternalValues_) {
                    _internal.timestamp = value.timestamp;
                }
            },
            enumerable: true
        }
    );

    // from
    Object.defineProperty(
        this,
        'from',
        {
            get: function () {return _internal.from;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.from = value.from;
            },
            enumerable: true
        }
    );

    // to
    Object.defineProperty(
        this,
        'to',
        {
            get: function () {return _internal.to;},
            set: function (value) {
                if (value instanceof InternalValues_) value = value.to;
                if (value instanceof Array) _internal.to = value;
            },
            enumerable: true
        }
    );

    // cc
    Object.defineProperty(
        this,
        'cc',
        {
            get: function () {return _internal.cc;},
            set: function (value) {
                if (value instanceof InternalValues_) value = value.cc;
                if (value instanceof Array) _internal.cc = value;
            },
            enumerable: true
        }
    );

    // bcc
    Object.defineProperty(
        this,
        'bcc',
        {
            get: function () {return _internal.bcc;},
            set: function (value) {
                if (value instanceof InternalValues_) value = value.bcc;
                if (value instanceof Array) _internal.bcc = value;
            },
            enumerable: true
        }
    );

    // body
    Object.defineProperty(
        this,
        'body',
        {
            get: function () {return _internal.body;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.body = new MessageBody(value.body);
                if (value instanceof MessageBody) _internal.body = value;
            },
            enumerable: true
        }
    );

    // isRead
    Object.defineProperty(
        this,
        'isRead',
        {
            get: function () {return _internal.isRead;},
            set: function (value) {
                if (value instanceof InternalValues_) {value = value.isRead;}
                _internal.isRead = !!value;
            },
            enumerable: true
        }
    );

    // hasAttachment
    Object.defineProperty(
        this,
        'hasAttachment',
        {
            get: function () {return _internal.attachments.length > 0;},
            set: function (value) {
                if (value instanceof InternalValues_)
                    _internal.hasAttachment = value.hasAttachment;
            },
            enumerable: true
        }
    );

    // isHighPriority
    Object.defineProperty(
        this,
        'isHighPriority',
        {
            get: function () {return _internal.isHighPriority;},
            set: function (value) {
                if (value instanceof InternalValues_) value = value.isHighPriority;
                _internal.isHighPriority = !!value;
            },
            enumerable: true
        }
    );

    // subject
    Object.defineProperty(
        this,
        'subject',
        {
            get: function () {return _internal.subject;},
            set: function (value) {
                if (value instanceof InternalValues_) value = value.subject;
                _internal.subject = String(value);
            },
            enumerable: true
        }
    );

    // inResponseTo
    Object.defineProperty(
        this,
        'inResponseTo',
        {
            get: function () {return _internal.inResponseTo;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.inResponseTo = value.inResponseTo;
            },
            enumerable: true
        }
    );

    // messageStatus
    Object.defineProperty(
        this,
        'messageStatus',
        {
            get: function () {
                if (_internal.id) {
                    // TODO create CPP layer
                    /*
                     *return bridge.sync({
                     *    cmd: 'Message_messageStatus',
                     *    args: {
                     *        id: _internal.id
                     *    }
                     *});
                     */
                    return _internal.messageStatus;
                } else {
                    return '';
                }
            },
            set: function (value) {return;},
            enumerable: true
        }
    );

    // attachments
    Object.defineProperty(
        this,
        'attachments',
        {
            get: function () {return _internal.attachments;},
            set: function(value) {
                if (value instanceof InternalValues_) {
                    value = value.attachments;
                    for (var k = 0; k < value.length; ++k) {
                        if (!(value[k] instanceof tizen.MessageAttachment)) {
                            if (_internal.attachments[k]) {
                                updateInternal_(_internal.attachments[k], value[k]);
                            } else {
                                _internal.attachments[k] = new MessageAttachment(
                                        new InternalValues_(value[k]));
                            }
                        } else {
                            _internal.attachments[k] = value[k];
                        }
                    }
                    // if new array is shorter than the old one, remove excess elements
                    if (value.length < _internal.length) {
                        _internal.splice(value.length, _internal.length - value.length);
                    }
                } else if (T_.isArray(value)) {
                    for (var k = 0; k < value.length; ++k) {
                        if (!(value[k] instanceof tizen.MessageAttachment)) {
                            return;
                        }
                    }
                    _internal.attachments = value;
                }
            },
            enumerable: true
        }
    );
};

function MessageInit(data) {
    if (!(this instanceof MessageInit)) {
        return new MessageInit(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }
    propertyFactory_(this, 'subject'       , data.subject        || ''   , Property.E | Property.W);
    propertyFactory_(this, 'to'            , data.to             || []   , Property.E | Property.W);
    propertyFactory_(this, 'cc'            , data.cc             || []   , Property.E | Property.W);
    propertyFactory_(this, 'bcc'           , data.bcc            || []   , Property.E | Property.W);
    propertyFactory_(this, 'plainBody'     , data.plainBody      || ''   , Property.E | Property.W);
    propertyFactory_(this, 'htmlBody'      , data.htmlBody       || ''   , Property.E | Property.W);
    propertyFactory_(this, 'isHighPriority', data.isHighPriority || false, Property.E | Property.W);
};

function MessageInit_(data) {
    if (!(this instanceof MessageInit_)) {
        return new MessageInit_(data);
    }
    if ( !data || typeof data !== 'object') {
        data = {};
    }
    this.id             = data.id             || null;
    this.conversationId = data.conversationId || null;
    this.folderId       = data.folderId       || null;
    this.timestamp      = data.timestamp      || null;
    this.from           = data.from           || '';
    this.to             = data.to             || [];
    this.cc             = data.cc             || [];
    this.bcc            = data.bcc            || [];
    this.isRead         = data.isRead         || false;
    this.hasAttachment  = data.hasAttachment  || null;
    this.isHighPriority = data.isHighPriority || false;
    this.subject        = data.subject        || '';
    this.inResponseTo   = data.inResponseTo   || null;
    this.attachments = [];
    this.plainBody      = data.body ? data.body.plainBody : '';
    this.htmlBody       = data.body ? data.body.htmlBody : '';

    var self = this;
    if (data.attachments && data.attachments.constructor === Array) {
        data.attachments.forEach(function(el) {
           if (!el) return;

           if (el.constructor === MessageAttachment) {
               self.attachments.push(el);
           } else {
               self.attachments.push(new MessageAttachment(new InternalValues_(el)));
           }
        });
    }
};

function MessageBody(data) {
    if (!this instanceof MessageBody) {
        return new MessageBody(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }

    var _internal = {
        messageId: data.messageId || null,
        loaded: data.loaded || false,
        plainBody: data.plainBody || '',
        htmlBody: data.htmlBody || '',
        inlineAttachments: data.inlineAttachments || []
    };

    // messageId
    Object.defineProperty(
        this,
        'messageId',
        {
            get: function () {return _internal.messageId;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.messageId = value.messageId;
            },
            enumerable: true
        }
    );

    // loaded
    Object.defineProperty(
        this,
        'loaded',
        {
            get: function () {return _internal.loaded;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.loaded = value.loaded;
            },
            enumerable: true
        }
    );

    // plainBody
    Object.defineProperty(
        this,
        'plainBody',
        {
            get: function () {return _internal.plainBody;},
            set: function (value) {
                if (value instanceof InternalValues_) {
                    _internal.plainBody = String(value.plainBody);
                } else {
                    _internal.plainBody = String(value);
                }
            },
            enumerable: true
        }
    );

    // htmlBody
    Object.defineProperty(
        this,
        'htmlBody',
        {
            get: function () {return _internal.htmlBody;},
            set: function (value) {
                if (value instanceof InternalValues_) {
                    _internal.htmlBody = String(value.htmlBody);
                } else {
                    _internal.htmlBody = String(value);
                }
            },
            enumerable: true
        }
    );

    // inlineAttachments
    Object.defineProperty(
        this,
        'inlineAttachments',
        {
            get: function () {return _internal.inlineAttachments;},
            set: function (value) {
                if (value instanceof InternalValues_) {
                    _internal.inlineAttachments = value.inlineAttachments;
                } else if (T_.isArray(value)) {
                    _internal.inlineAttachments = value;
                }
            },
            enumerable: true
        }
    );
};

var messageAttachmentsLoaded = {};

function MessageAttachment(first, second) {
    validator_.isConstructorCall(this, MessageAttachment);
    if (!this instanceof MessageAttachment) {
        return new MessageAttachment(data);
    }

    var internalConstructor = first instanceof InternalValues_;
    var _internal = {
        messageId: internalConstructor ? first.messageId : null,
        id: internalConstructor ? first.id : null,
        mimeType: internalConstructor ? first.mimeType : (undefined == second ? null : second),
        filePath: internalConstructor ? first.filePath : first,
    };

    // messageId
    Object.defineProperty(
        this,
        'messageId',
        {
            get: function () {return _internal.messageId;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.messageId = value.messageId;
            },
            enumerable: true
        }
    );
    // id
    Object.defineProperty(
        this,
        'id',
        {
            get: function () {return _internal.id;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.id = value.id;
            },
            enumerable: true
        }
    );
    // mimeType
    Object.defineProperty(
        this,
        'mimeType',
        {
            get: function () {return _internal.mimeType;},
            set: function (value) {
                if (value instanceof InternalValues_) _internal.mimeType = value.mimeType;
            },
            enumerable: true
        }
    );
    // filePath
    Object.defineProperty(
        this,
        'filePath',
        {
            get: function () {
                if (_internal.id && !messageAttachmentsLoaded[_internal.id]) {
                    return null;
                }

                return _internal.filePath;
            },
            set: function (value) {
                if (value instanceof InternalValues_) _internal.filePath = value.filePath;
            },
            enumerable: true
        }
    );
};

function Messaging() {};

/**
 * Gets the messaging service of a given type for a given account.
 * @param {!MessageServiceTag} messageServiceType Type of the services to be retrieved.
 * @param {!MessageServiceArraySuccessCallback} successCallback Callback function that is called
 *     when the services are successfully retrieved.
 * @param {ErrorCallback} errorCallback Callback function that is called when an error occurs.
 */
Messaging.prototype.getMessageServices = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'messageServiceType', type: types_.ENUM, values: MessageServiceTag},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    bridge.async({
        cmd: 'Messaging_getMessageServices',
        args: {
            messageServiceType: args.messageServiceType
        }
    }).then({
        success: function (data) {
            var servicesArr = [];
            data.forEach(function(e){
                servicesArr.push(new MessageService(e));
            });
            args.successCallback.call(null, servicesArr);
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
}
function MessageStorage(){};
function MessageService(data) {
    propertyFactory_(this, 'id', data.id, Property.E);
    propertyFactory_(this, 'type', data.type, Property.E);
    propertyFactory_(this, 'name', data.name, Property.E);
    propertyFactory_(this, 'messageStorage', new MessageStorage(this), Property.E);
};

MessageService.prototype.sendMessage = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'simIndex', type: types_.LONG, optional: true, nullable: true}
    ]);

    if (args.message.type != this.type) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }

    var self = this;
    bridge.async({
        cmd: 'MessageService_sendMessage',
        args: {
            message: args.message,
            simIndex: args.simIndex || 1,
            serviceId: self.id
        }
    }).then({
        success: function (data) {
            var message = data.message;
            if (message) {
                var body = message.body;
                if (body) {
                    updateInternal_(args.message.body, body)
                    delete message.body;
                }
                updateInternal_(args.message, message);
            }

            if (args.successCallback) {
                args.successCallback.call(null, data.recipients);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageService.prototype.loadMessageBody = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    if (args.message.type != this.type) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }

    var self = this;

    bridge.async({
        cmd: 'MessageService_loadMessageBody',
        args: {
            message: args.message,
            serviceId: self.id
        }
    }).then({
        success: function (data) {
            var body = data.messageBody;
            if (body) {
                updateInternal_(args.message.body, body)
            }

            args.successCallback.call(
                null,
                args.message
            );
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};
MessageService.prototype.loadMessageAttachment = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'attachment', type: types_.PLATFORM_OBJECT, values: MessageAttachment},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;
    var firstCall = false;
    if (!messageAttachmentsLoaded[args.attachment.id]) {
        firstCall = true;
        messageAttachmentsLoaded[args.attachment.id] = true;
    }

    bridge.async({
        cmd: 'MessageService_loadMessageAttachment',
        args: {
            attachment: args.attachment,
            serviceId: self.id
        }
    }).then({
        success: function (data) {
            if (args.successCallback) {
                var messageAttachment = data.messageAttachment;
                if (messageAttachment) {
                    updateInternal_(args.attachment, messageAttachment);
                }

                args.successCallback.call(
                    null,
                    args.attachment
                );
            }
        },
        error: function (e) {
            if (firstCall) {
                messageAttachmentsLoaded[args.attachment.id] = false;
            }
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageService.prototype.sync = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'limit', type: types_.UNSIGNED_LONG, optional: true, nullable: true}
    ]);

    var self = this;
    var cid = bridge.listener({
        success: function () {
            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });

    var result = bridge.sync({
        cmd: 'MessageService_sync',
        cid: cid,
        args: {
            id: self.id,
            limit: args.limit || null
        }
    });

    return result;
};

MessageService.prototype.syncFolder = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'folder', type: types_.PLATFORM_OBJECT, values: MessageFolder},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'limit', type: types_.UNSIGNED_LONG, optional: true, nullable: true}
    ]);

    var self = this;
    var cid = bridge.listener({
        success: function () {
            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });

    var result = bridge.sync({
        cmd: 'MessageService_syncFolder',
        cid: cid,
        args: {
            id: self.id,
            folder: args.folder,
            limit: args.limit || null
        }
    });

    return result;
};

MessageService.prototype.stopSync = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'opId', type: types_.LONG}
    ]);

    var self = this;
    bridge.sync({
        cmd: 'MessageService_stopSync',
        args: {
            id: self.id,
            opId: args.opId
        }
    });
};

function MessageStorage(service) {
    propertyFactory_(this, 'service', service);
};

MessageStorage.prototype.addDraftMessage = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    if (args.message.type != this.service.type) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }

    var self = this;
    bridge.async({
        cmd: 'MessageStorage_addDraftMessage',
        args: {
            message: args.message,
            serviceId: self.service.id
        }
    }).then({
        success: function (data) {
            var message = data.message;
            if (message) {
                var body = message.body;
                if (body) {
                    updateInternal_(args.message.body, body)
                    delete message.body;
                }
                var attachments = message.attachments;
                if (attachments) {
                    for (var i = 0; i < attachments.length; i++) {
                        messageAttachmentsLoaded[attachments[i].id] = true;
                    }
                }
                updateInternal_(args.message, message);
            }

            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.findMessages = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter]
        },
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'sort', type: types_.PLATFORM_OBJECT, values: tizen.SortMode, optional: true,
                nullable: true},
        {name: 'limit', type: types_.UNSIGNED_LONG, optional: true, nullable: true},
        {name: 'offset', type: types_.UNSIGNED_LONG, optional: true, nullable: true}
    ]);

    var self = this;

    bridge.async({
        cmd: 'MessageStorage_findMessages',
        args: {
            filter: addTypeToFilter_(args.filter) || null,
            sort: args.sort || null,
            limit: args.limit || null,
            offset: args.offset || null,
            serviceId: self.service.id,
            type: self.service.type
        }
    }).then({
        success: function (data) {
            var messages = [];
            data.forEach(function (el) {
                messages.push(new tizen.Message(el.type, new MessageInit_(el)));
            });
            args.successCallback.call(null, messages);
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.removeMessages = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'messages', type: types_.ARRAY, values: Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

    args.messages.forEach(function(msg) {
        if (msg.type != self.service.type) {
            throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
        }
    });

    bridge.async({
        cmd: 'MessageStorage_removeMessages',
        args: {
            messages: args.messages,
            serviceId: self.service.id,
            type: self.service.type
        }
    }).then({
        success: function () {
            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.updateMessages = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'messages', type: types_.ARRAY, values: Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

    args.messages.forEach(function(msg) {
        if (msg.type != self.service.type) {
            throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
        }
    });

    bridge.async({
        cmd: 'MessageStorage_updateMessages',
        args: {
            messages: args.messages,
            serviceId: self.service.id
        }
    }).then({
        success: function (data) {
            var originals = {},
                    i = args.messages.length,
                    m;
            while (i--) {
                m = args.messages[i];
                if (m.id) {
                    originals[m.id] = m;
                }
            }

            i = data.length;
            while (i--) {
                m = data[i];
                if (originals[m.oldId]) {
                    var body = m.body;
                    if (body) {
                        updateInternal_(originals[m.oldId].body, body)
                        delete m.body;
                    }
                    updateInternal_(originals[m.oldId], m);
                }
            }

            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.findConversations = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter]
        },
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'sort', type: types_.PLATFORM_OBJECT, values: tizen.SortMode, optional: true,
                nullable: true},
        {name: 'limit', type: types_.UNSIGNED_LONG, optional: true, nullable: true},
        {name: 'offset', type: types_.UNSIGNED_LONG, optional: true, nullable: true}
    ]);

    var self = this;

    bridge.async({
        cmd: 'MessageStorage_findConversations',
        args: {
            filter: addTypeToFilter_(args.filter),
            sort: args.sort || null,
            limit: args.limit || null,
            offset: args.offset || null,
            serviceId: self.service.id
        }
    }).then({
        success: function (data) {
            var conversations = [];
            data.forEach(function (el) {
                conversations.push(new MessageConversation(el));
            });
            args.successCallback.call(null, conversations);
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.removeConversations = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_WRITE);

    var args = validator_.validateArgs(arguments, [
        {name: 'conversations', type: types_.ARRAY},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    args.conversations.forEach(function (el) {
        if (!el || el.constructor !== MessageConversation) {
            throw new WebAPIException(WebAPIException.TYPE_MISMATCH_ERR);
        }
    });

    var self = this;

    bridge.async({
        cmd: 'MessageStorage_removeConversations',
        args: {
            conversations: args.conversations,
            serviceId: self.service.id,
            type: self.service.type
        }
    }).then({
        success: function () {
            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.findFolders = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter]
        },
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

    bridge.async({
        cmd: 'MessageStorage_findFolders',
        args: {
            filter: addTypeToFilter_(args.filter),
            sort: args.sort || null,
            limit: args.limit || null,
            offset: args.offset || null,
            serviceId: self.service.id
        }
    }).then({
        success: function (data) {
            var folders = [];
            data.forEach(function (el) {
                folders.push(new MessageFolder(el));
            });
            args.successCallback.call(null, folders);
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new WebAPIException(e.error)
                )
            }
        }
    });
};

MessageStorage.prototype.addMessagesChangeListener = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {name: 'messagesChangeCallback', type: types_.LISTENER,
                values: ['messagesadded', 'messagesupdated', 'messagesremoved']},
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
            optional: true,
            nullable: true
        }
    ]);

    var self = this;

    var cid = bridge.listener({
        messagesadded: function (data) {
            if (args.messagesChangeCallback.messagesadded) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el.type, new MessageInit_(el)));
                });
                args.messagesChangeCallback.messagesadded.call(null, messages);
            }
        },
        messagesupdated: function (data) {
            if (args.messagesChangeCallback.messagesupdated) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el.type, new MessageInit_(el)));
                });
                args.messagesChangeCallback.messagesupdated.call(null, messages);
            }
        },
        messagesremoved: function (data) {
            if (args.messagesChangeCallback.messagesremoved) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el.type, new MessageInit_(el)));
                });
                args.messagesChangeCallback.messagesremoved.call(null, messages);
            }
        }
    });

    var result = bridge.sync({
        cmd: 'MessageStorage_addMessagesChangeListener',
        cid: cid,
        args: {
            filter: args.filter ? addTypeToFilter_(args.filter) : null,
            serviceId: self.service.id
        }
    });

    bridge.attach(cid, 'watchId', result);
    return result;
};

MessageStorage.prototype.addConversationsChangeListener = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {name: 'conversationsChangeCallback', type: types_.LISTENER,
                values: ['conversationsadded', 'conversationsupdated', 'conversationsremoved']},
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
            optional: true,
            nullable: true
        }
    ]);

    var self = this;

    var cid = bridge.listener({
        conversationsadded: function (data) {
            if (args.conversationsChangeCallback.conversationsadded) {
                var conversations = [];
                data.forEach(function (el) {
                    conversations.push(new MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsadded.call(null, conversations);
            }
        },
        conversationsupdated: function (data) {
            if (args.conversationsChangeCallback.conversationsupdated) {
                var conversations = [];
                data.forEach(function (el) {
                   conversations.push(new MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsupdated.call(null, conversations);
            }
        },
        conversationsremoved: function (data) {
            if (args.conversationsChangeCallback.conversationsremoved) {
                var conversations = [];
                data.forEach(function (el) {
                    conversations.push(new MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsremoved.call(null, conversations);
            }
        }
    });

    var result = bridge.sync({
        cmd: 'MessageStorage_addConversationsChangeListener',
        cid: cid,
        args: {
            filter: args.filter ? addTypeToFilter_(args.filter) : null,
            serviceId: self.service.id
        }
    });

    bridge.attach(cid, 'watchId', result);
    return result;
};

MessageStorage.prototype.addFoldersChangeListener = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {name: 'foldersChangeCallback', type: types_.LISTENER,
                values: ['foldersadded', 'foldersupdated', 'foldersremoved']},
        {
            name: 'filter',
            type: types_.PLATFORM_OBJECT,
            values: [tizen.AttributeFilter, tizen.AttributeRangeFilter, tizen.CompositeFilter],
            optional: true,
            nullable: true
        }
    ]);

    var self = this;

    var cid = bridge.listener({
        foldersadded: function (data) {
            if (args.foldersChangeCallback.foldersadded) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new MessageFolder(el));
                });
                args.foldersChangeCallback.foldersadded.call(null, folders);
            }
        },
        foldersupdated: function (data) {
            if (args.foldersChangeCallback.foldersupdated) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new MessageFolder(el));
                });
                args.foldersChangeCallback.foldersupdated.call(null, folders);
            }
        },
        foldersremoved: function (data) {
            if (args.foldersChangeCallback.foldersremoved) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new MessageFolder(el));
                });
                args.foldersChangeCallback.foldersremoved.call(null, folders);
            }
        }
    });

    var result = bridge.sync({
        cmd: 'MessageStorage_addFoldersChangeListener',
        cid: cid,
        args: {
            filter: args.filter ? addTypeToFilter_(args.filter) : null,
            serviceId: self.service.id
        }
    });

    bridge.attach(cid, 'watchId', result);
    return result;
};

MessageStorage.prototype.removeChangeListener = function () {
    xwalk.utils.checkPrivilegeAccess(xwalk.utils.privilege.MESSAGING_READ);

    var args = validator_.validateArgs(arguments, [
        {name: 'watchId', type: types_.LONG}
    ]);

    var self = this;

    var result = bridge.sync({
        cmd: 'MessageStorage_removeChangeListener',
        args: {
            watchId: args.watchId,
            serviceId: self.service.id
        }
    });

    bridge.find('watchId', args.watchId).forEach(function (e) {
        bridge.remove(e.id);
    });
    return result;
};

function MessageConversation(data) {
    propertyFactory_(this, 'id'            , data.id             || null , Property.E);
    propertyFactory_(this, 'type'          , data.type           || ''   , Property.E);
    propertyFactory_(this, 'timestamp'     , data.timestamp ? new Date(data.timestamp * 1000) : null , Property.E);
    propertyFactory_(this, 'messageCount'  , data.messageCount   || 0    , Property.E);
    propertyFactory_(this, 'unreadMessages', data.unreadMessages || 0    , Property.E);
    propertyFactory_(this, 'preview'       , data.preview        || ''   , Property.E);
    propertyFactory_(this, 'subject'       , data.subject        || ''   , Property.E);
    propertyFactory_(this, 'isRead'        , data.isRead         || false, Property.E);
    propertyFactory_(this, 'from'          , data.from           || null , Property.E);
    propertyFactory_(this, 'to'            , data.to             || []   , Property.E);
    propertyFactory_(this, 'cc'            , data.cc             || []   , Property.E);
    propertyFactory_(this, 'bcc'           , data.bcc            || []   , Property.E);
    propertyFactory_(this, 'lastMessageId' , data.lastMessageId  || null , Property.E);
};

function MessageFolder(data) {
    var _internal = {
            id: data.id || null,
            parentId: data.parentId || null,
            serviceId: data.serviceId || '',
            contentType: data.contentType || '',
            name: data.name || '',
            path: data.path || '',
            type: data.type || '',
            synchronizable: data.synchronizable || false
        };

        Object.defineProperty(
            this,
            'id',
            {
                get: function () {return _internal.id;},
                enumerable: true
            }
        );

        Object.defineProperty(
                this,
                'parentId',
                {
                    get: function () {return _internal.parentId;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'serviceId',
                {
                    get: function () {return _internal.serviceId;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'contentType',
                {
                    get: function () {return _internal.contentType;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'name',
                {
                    get: function () {return _internal.name;},
                    set: function (value) { if (value) _internal.name = value;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'path',
                {
                    get: function () {return _internal.path;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'type',
                {
                    get: function () {return _internal.type;},
                    enumerable: true
                }
        );

        Object.defineProperty(
                this,
                'synchronizable',
                {
                    get: function () {return _internal.synchronizable;},
                    set: function (value) { _internal.synchronizable = Boolean(value);},
                    enumerable: true
                }
        );
};

tizen.Message = Message;

tizen.MessageAttachment = MessageAttachment;

exports = new Messaging();
