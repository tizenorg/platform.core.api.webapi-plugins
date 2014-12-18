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
    if (data === null || typeof data !== 'object') { // 'data' is optional
        data = {};
    }
    // set initial data from internal MessageInit_ object or to default values
    var internal       = data instanceof MessageInit_,
        id             = internal ? data.id             : null,
        conversationId = internal ? data.conversationId : null,
        folderId       = internal ? data.folderId       : null,
        timestamp      = internal ? data.timestamp      : null,
        from           = internal ? data.from           : null,
        isRead         = internal ? data.isRead         : false,
        inResponseTo   = internal ? data.inResponseTo   : null,
        messageStatus  = internal ? data.messageStatus  : '';
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
    // set properties
    propertyFactory_(this, 'id'            , id                  || null , Property.E             );
    propertyFactory_(this, 'conversationId', conversationId      || null , Property.E             );
    propertyFactory_(this, 'folderId'      , folderId            || null , Property.E             );
    propertyFactory_(this, 'type'          , type                || null , Property.E             );
    propertyFactory_(this, 'timestamp'     , timestamp           || null , Property.E             );
    propertyFactory_(this, 'from'          , from                || null , Property.E             );
    propertyFactory_(this, 'to'            , to                  || []   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'cc'            , cc                  || []   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'bcc'           , bcc                 || []   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'body'          , body                        , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'isRead'        , isRead              || false, Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'isHighPriority', data.isHighPriority || false, Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'inResponseTo'  , inResponseTo        || null , Property.E             ); // TODO: setraises
    propertyFactory_(this, 'messageStatus' , messageStatus       || ''   , Property.E             );
    // 'attachments' private variable, getter and setter
    var attachments = (internal ? data.attachments : []) || [];
    propertyFactory_(
        this,
        'attachments',
        undefined,
        Property.E,
        {
            get: function() {
                return attachments;
            },
            set: function(newattachments) {
                for (var k = 0; k < newattachments.length; ++k) {
                    if (!(newattachments[k] instanceof tizen.MessageAttachment)) {
                        return;
                    }
                }
                attachments = newattachments;
            }
        }
    );
    // 'subject' private variable, getter and setter
    var subject = data.subject || '';
    propertyFactory_(
        this,
        'subject',
        undefined,
        Property.E,
        {
            get: function() {
                return subject;
            },
            set: function(newsubject) {
                if (typeof newsubject !== 'string') {
                    subject = '';
                    return;
                }
                subject = newsubject;
            }
        }
    );
    // 'hasAttachment' getter
    propertyFactory_(
        this,
        'hasAttachment',
        undefined,
        Property.E,
        {
            get: function() {
                return this.attachments.length > 0;
            }
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
    if (data === null || typeof data !== 'object') {
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
    this.body           = data.body           || new MessageBody();
    this.isRead         = data.isRead         || false;
    this.hasAttachment  = data.hasAttachment  || null;
    this.isHighPriority = data.isHighPriority || false;
    this.subject        = data.subject        || '';
    this.inResponseTo   = data.inResponseTo   || null;
    this.messageStatus  = data.messageStatus  || '';
    this.attachments    = data.attachments    || [];
};

function MessageBody(data) {
    if (!this instanceof MessageBody) {
        return new MessageBody(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }
    propertyFactory_(this, 'messageId'        , data.messageId         || null , Property.E             );
    propertyFactory_(this, 'loaded'           , data.loaded            || false, Property.E             );
    propertyFactory_(this, 'plainBody'        , data.plainBody         || ''   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'htmlBody'         , data.htmlBody          || ''   , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'inlineAttachments', data.inlineAttachments || []   , Property.E | Property.W); // TODO: setraises
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
    console.dir(this);
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

    bridge.async({
        cmd: 'MessageService_sendMessage',
        args: {
            message: args.message,
            simIndex: args.simIndex
        }
    }).then({
        success: function (data) {
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

    bridge.async({
        cmd: 'MessageService_loadMessageBody',
        args: {
            message: args.message
        }
    }).then({
        success: function (data) {
            args.successCallback.call(
                null,
                new tizen.Message(data.type, new MessageInit_(data.messageInit))
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
        {name: 'attachment', type: types_.PLATFORM_OBJECT, values: tizen.MessageAttachment},
        {name: 'successCallback', type: types_.FUNCTION},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    bridge.async({
        cmd: 'MessageService_loadMessageAttachment',
        args: {
            attachment: args.attachment
        }
    }).then({
        success: function (data) {
            if (args.successCallback) {
                // TODO problem with MessageAttachment Constructor need to be investigated
                args.successCallback.call(
                    null,
                    new MessageAttachment(data.filePath, data.mimeType)
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

    bridge.async({
        cmd: 'MessageService_syncFolder',
        args: {
            folder: args.folder,
            limit: args.limit
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

    bridge.async({
        cmd: 'MessageStorage_addDraftMessage',
        args: {
            message: args.message,
            serviceId: this.service.id
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

    bridge.async({
        cmd: 'MessageStorage_findMessages',
        args: {
            filter: args.filter,
            sort: args.sort,
            limit: args.limit,
            offset: args.offset
        }
    }).then({
        success: function (data) {
            var messages = [];
            data.forEach(function (el) {
                messages.push(new tizen.Message(el));
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

    bridge.async({
        cmd: 'MessageStorage_removeMessages',
        args: {
            messages: args.messages
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

    bridge.async({
        cmd: 'MessageStorage_updateMessages',
        args: {
            messages: args.messages
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

    bridge.async({
        cmd: 'MessageStorage_findConversations',
        args: {
            filter: args.filter,
            sort: args.sort,
            limit: args.limit,
            offset: args.offset
        }
    }).then({
        success: function (data) {
            var conversations = [];
            data.forEach(function (el) {
                conversations.push(new MessageConversation(el));
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

MessageStorage.prototype.removeConversations = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'conversations', type: types_.ARRAY},
        {name: 'successCallback', type: types_.FUNCTION, optional: true, nullable: true},
        {name: 'errorCallback', type: types_.FUNCTION, optional: true, nullable: true}
    ]);

    bridge.async({
        cmd: 'MessageStorage_removeConversations',
        args: {
            conversations: args.conversations
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

    bridge.async({
        cmd: 'MessageStorage_findFolders',
        args: {
            filter: args.filter,
            sort: args.sort,
            limit: args.limit,
            offset: args.offset
        }
    }).then({
        success: function (data) {
            var conversations = [];
            data.forEach(function (el) {
                conversations.push(new MessageConversation(el));
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

MessageStorage.prototype.addMessagesChangeListener = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'messagesChangeCallback', type: types_.LISTENER,
                values: ['messagesadded', 'messagesupdated', 'messagesremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
    ]);

    var listeners = [];
    if (args.messagesChangeCallback.messagesadded) listeners.push('messagesadded');
    if (args.messagesChangeCallback.messagesupdated) listeners.push('messagesupdated');
    if (args.messagesChangeCallback.messagesremoved) listeners.push('messagesremoved');

    bridge({
        cmd: 'MessageStorage_addMessagesChangeListener',
        args: {
            filter: args.filter,
            listeners: listeners
        }
    }).then({
        messagesadded: function (data) {
            if (args.messagesChangeCallback.messagesadded) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el));
                });
                args.messagesChangeCallback.messagesadded.call(null, messages);
            }
        },
        messagesupdated: function (data) {
            if (args.messagesChangeCallback.messagesupdated) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el));
                });
                args.messagesChangeCallback.messagesupdated.call(null, messages);
            }
        },
        messagesremoved: function (data) {
            if (args.messagesChangeCallback.messagesremoved) {
                var messages = [];
                data.forEach(function (el) {
                    messages.push(new tizen.Message(el));
                });
                args.messagesChangeCallback.messagesremoved.call(null, messages);
            }
        }
    });
};

MessageStorage.prototype. addConversationsChangeListener = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'conversationsChangeCallback', type: types_.LISTENER,
                values: ['conversationsadded', 'conversationsupdated', 'conversationsremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
    ]);

    var listeners = [];
    if (args.conversationsChangeCallback.conversationsadded)
            listeners.push('conversationsadded');
    if (args.conversationsChangeCallback.conversationsupdated)
            listeners.push('conversationsupdated');
    if (args.conversationsChangeCallback.conversationsremoved)
            listeners.push('conversationsremoved');

    bridge({
        cmd: 'MessageStorage_addConversationsChangeListener',
        args: {
            filter: args.filter,
            listeners: listeners
        }
    }).then({
        conversationsadded: function (data) {
            if (args.conversationsChangeCallback.conversationsadded) {
                var conversations = [];
                data.forEach(function (el) {
                    conversations.push(new tizen.MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsadded.call(null, conversations);
            }
        },
        conversationsupdated: function (data) {
            if (args.conversationsChangeCallback.conversationsupdated) {
                var conversations = [];
                data.forEach(function (el) {
                   conversations.push(new tizen.MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsupdated.call(null, conversations);
            }
        },
        conversationsremoved: function (data) {
            if (args.conversationsChangeCallback.conversationsremoved) {
                var conversations = [];
                data.forEach(function (el) {
                    conversations.push(new tizen.MessageConversation(el));
                });
                args.conversationsChangeCallback.conversationsremoved.call(null, conversations);
            }
        }
    });
};

MessageStorage.prototype.addFoldersChangeListener = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'foldersChangeCallback', type: types_.LISTENER,
                values: ['foldersadded', 'foldersupdated', 'foldersremoved']},
        {name: 'filter', type: types_.PLATFORM_OBJECT, values: tizen.AbstractFilter,
                optional: true, nullable: true}
    ]);
    var listeners = [];
    if (args.foldersChangeCallback.foldersadded) listeners.push('foldersadded');
    if (args.foldersChangeCallback.foldersupdated) listeners.push('foldersupdated');
    if (args.foldersChangeCallback.foldersremoved) listeners.push('foldersremoved');

    bridge({
        cmd: 'MessageStorage_addFoldersChangeListener',
        args: {
            filter: args.filter,
            listeners: listeners
        }
    }).then({
        foldersadded: function (data) {
            if (args.foldersChangeCallback.foldersadded) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new tizen.MessageFolder(el));
                });
                args.foldersChangeCallback.foldersadded.call(null, folders);
            }
        },
        foldersupdated: function (data) {
            if (args.foldersChangeCallback.foldersupdated) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new tizen.MessageFolder(el));
                });
                args.foldersChangeCallback.foldersupdated.call(null, folders);
            }
        },
        foldersremoved: function (data) {
            if (args.foldersChangeCallback.foldersremoved) {
                var folders = [];
                data.forEach(function (el) {
                    folders.push(new tizen.MessageFolder(el));
                });
                args.foldersChangeCallback.foldersremoved.call(null, folders);
            }
        }
    });
};

MessageStorage.prototype.removeChangeListener = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'watchId', type: types_.LONG}
    ]);

    bridge.sync({
        cmd: 'MessageStorage_removeChangeListener',
        args: {
            watchId: args.watchId
        }
    });
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
