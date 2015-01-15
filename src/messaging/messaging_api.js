//@ sourceURL=messaging_api.js

// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;
var bridge = xwalk.utils.NativeBridge(extension, true);

function throwException_(err) {
    throw new tizen.WebAPIException(err.code, err.name, err.message);
}

var Property = {
    W: 1 << 0,   // WRITABLE
    E: 1 << 1,   // ENUMERABLE
    C: 1 << 2    // CONFIGURABLE
}

//TODO remove CommonFS when C++ filesystem will be available
function CommonFS(){};
CommonFS.cacheVirtualToReal = {
        'downloads' : { path: '/opt/usr/media/Downloads'},
        'documents' : { path: '/opt/usr/media/Documents'},
        'music'     : { path: '/opt/usr/media/Sounds'},
        'images'    : { path: '/opt/usr/media/Images'},
        'videos'    : { path: '/opt/usr/media/Videos'},
        'ringtones' : { path: '/opt/usr/share/settings/Ringtones'}
};

CommonFS.toRealPath = function (aPath) {
    var _fileRealPath = '',
        _uriPrefix = 'file://',
        i;
    if (aPath.indexOf(_uriPrefix) === 0) {
        _fileRealPath = aPath; /*.substr(_uriPrefix.length);*/
    } else if (aPath[0] != '/') {
        //virtual path$
        var _pathTokens = aPath.split('/');
        if (this.cacheVirtualToReal[_pathTokens[0]] && (
                this.cacheVirtualToReal[_pathTokens[0]].state === undefined ||
                this.cacheVirtualToReal[_pathTokens[0]].state === 'MOUNTED')) {
            _fileRealPath = this.cacheVirtualToReal[_pathTokens[0]].path;
            for (i = 1; i < _pathTokens.length; ++i) {
                _fileRealPath += '/' + _pathTokens[i];
            }
        } else {
            _fileRealPath = aPath;
        }
    } else {
        _fileRealPath = aPath;
    }
    return _fileRealPath;
};

function addTypeToFilter_(data)
{
    var filter = {};

    for(var field in data) {
        filter[field] = data[field];
    }

    if (data instanceof tizen.AttributeFilter) {
        filter.type = "AttributeFilter";
    }
    if (data instanceof tizen.AttributeRangeFilter) {
        filter.type = "AttributeRangeFilter";
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
        if (data.hasOwnProperty(key) && internal.hasOwnProperty(key)) {
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
        return new Message(type, data);
    }
    if (MessageServiceTag.indexOf(type) === -1) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
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
        from: from || '',
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
                return _internal.timestamp ? new Date(_internal.timestamp) : _internal.timestamp;
            },
            set: function (value) {
                if (value instanceof InternalValues_) value = value.timestamp;
                _internal.timestamp = value;
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
                _internal.isHighPriority = value;
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
                if (typeof value !== 'string') return;
                _internal.subject = value;
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
                if (value instanceof InternalValues_) value = value.attachments;
                for (var k = 0; k < value.length; ++k) {
                    if (!(value[k] instanceof tizen.MessageAttachment)) {
                        return;
                    }
                }
                _internal.attachments = value;
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
    this.from           = data.from           || null;
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
               self.attachments.push(new MessageAttachment_(el));
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
                } else {
                    _internal.inlineAttachments = value;
                }
            },
            enumerable: true
        }
    );
};

function MessageAttachment_(data) {
    if (!(this instanceof MessageAttachment_)) return new MessageAttachment_(data);
    propertyFactory_(this, 'id'       , data.id       , Property.E);
    propertyFactory_(this, 'messageId', data.messageId, Property.E);

    var attachment = MessageAttachment.apply(this, [data.filePath, data.mimeType]);
    attachment.constructor = MessageAttachment;

    return attachment;
}

function MessageAttachment(filePath, mimeType) {
    //TODO remove CommonFS.toRealPath function when C++ filesystem will be available
    filePath = CommonFS.toRealPath(filePath);
    if (!this.id) {
        propertyFactory_(this, 'id', null, Property.E);
    }
    if (!this.messageId) {
        propertyFactory_(this, 'messageId', null, Property.E);
    }
    propertyFactory_(this, 'mimeType', mimeType || '', Property.E);
    propertyFactory_(this, 'filePath', filePath || '', Property.E);

    return this;
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
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
    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'simIndex', type: types_.LONG, optional: true, nullable: true}
    ]);

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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageService.prototype.loadMessageBody = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};
MessageService.prototype.loadMessageAttachment = function () {
    var args = validator_.validateArgs(arguments, [
        //TODO FIXME something wrong with MessageAttachment object constructor
        // fix validation of first argument
        {name: 'attachment', type: types_.PLATFORM_OBJECT, values: Object},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

    bridge.async({
        cmd: 'MessageService_loadMessageAttachment',
        args: {
            attachment: args.attachment,
            serviceId: self.id
        }
    }).then({
        success: function (data) {
            if (args.successCallback) {
                // TODO problem with MessageAttachment Constructor need to be investigated
                args.successCallback.call(
                    null,
                    new MessageAttachment_(data.messageAttachment)
                );
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageService.prototype.sync = function () {
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
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
    var args = validator_.validateArgs(arguments, [
        {name: 'message', type: types_.PLATFORM_OBJECT, values: tizen.Message},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.findMessages = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter},
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.removeMessages = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'messages', type: types_.ARRAY},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.updateMessages = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'messages', type: types_.ARRAY},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    var self = this;

    bridge.async({
        cmd: 'MessageStorage_updateMessages',
        args: {
            messages: args.messages,
            serviceId: self.service.id
        }
    }).then({
        success: function (data) {
            var originals = {};
            args.messages.forEach(function (m) {
                if (m.id) {
                    originals[m.id] = m;
                }
            });
            data.forEach(function (message) {
                if (!originals[message.id]) {return;}
                var body = message.body;
                if (body) {
                    updateInternal_(originals[message.id].body, body)
                    delete message.body;
                }
                updateInternal_(originals[message.id], message);
            });

            if (args.successCallback) {
                args.successCallback.call(null);
            }
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.findConversations = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter},
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.removeConversations = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'conversations', type: types_.ARRAY},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    args.conversations.forEach(function (el) {
        if (!el || el.constructor !== MessageConversation) {
            throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.findFolders = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter},
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
                    new tizen.WebAPIException(e.code, e.message, e.name)
                )
            }
        }
    });
};

MessageStorage.prototype.addMessagesChangeListener = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'messagesChangeCallback', type: types_.LISTENER,
                values: ['messagesadded', 'messagesupdated', 'messagesremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
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
    var args = validator_.validateArgs(arguments, [
        {name: 'conversationsChangeCallback', type: types_.LISTENER,
                values: ['conversationsadded', 'conversationsupdated', 'conversationsremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
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
    var args = validator_.validateArgs(arguments, [
        {name: 'foldersChangeCallback', type: types_.LISTENER,
                values: ['foldersadded', 'foldersupdated', 'foldersremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
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
    propertyFactory_(this, 'timestamp'     , data.timestamp      || null , Property.E);
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
    propertyFactory_(this, 'id'            , data.id             || null , Property.E             );
    propertyFactory_(this, 'parentId'      , data.parentId       || null , Property.E             );
    propertyFactory_(this, 'serviceId'     , data.serviceId      || ''   , Property.E             );
    propertyFactory_(this, 'contentType'   , data.contentType    || ''   , Property.E             );
    propertyFactory_(this, 'name'          , data.name           || ''   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'path'          , data.path           || ''   , Property.E             );
    propertyFactory_(this, 'type'          , data.type           || ''   , Property.E             );
    propertyFactory_(this, 'synchronizable', data.synchronizable || false, Property.E | Property.W); // TODO: setraises
};

tizen.Message = Message;

tizen.MessageAttachment = MessageAttachment;

exports = new Messaging();
