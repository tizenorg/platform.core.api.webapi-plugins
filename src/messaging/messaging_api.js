// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

var validator_ = xwalk.utils.validator;
var types_ = validator_.Types;

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
function propertyFactory_(that, name, value, flags) {
    flags = flags || 0;
    var options = {value: value, writable: false, enumerable: false, configurable: false};
    if ((flags & Property.W) === Property.W) options.writable = true;
    if ((flags & Property.E) === Property.E) options.enumerable = true;
    if ((flags & Property.C) === Property.C) options.configurable = true;

    Object.defineProperty(
        that,
        name,
        options
    );
}

/*
 *bridge is a two way communication interface
 *Example usage:
 *    To send sync method:
 *    var result = bridge.sync({
 *        cmd: 'my_cpp_function_symbol',
 *        data: {
 *            name: 'My name',
 *            age: 28
 *        }
 *    });
 *    console.log(result);
 *
 *    To send async method and handle response:
 *    bridge.async({
 *        cmd: 'my_cpp_function_symbol',
 *        data: {
 *            name: 'My name'
 *        }
 *    }).then({
 *        success: function (data) {
 *            var age = data.age;
 *            args.successCallback(age);
 *        },
 *        error: function (e) {...},
 *        someCallback: function (data) {...}
 *    });
 *bridge.async will add special param to passed data called cid
 *that param need to be kept and returned with respons
 *To determine which callback should be invoked, response should
 *contain "action" param. Value of "action" param indicates name of
 *triggered callback.
 *Callbask are removed from listenr by defoult to prevent that behaviour
 *param "keep" should be assigned to value true
 *Example of c++ async response:
 *    Simple succes with data:
 *    {
 *        cid: 23,
 *        action: 'success',
 *        data: {
 *            age: 23
 *        }
 *    }
 *    More complicated example:
 *    {
 *        cid: 23,
 *        action: 'progress',
 *        keep: true,
 *        data: {
 *            age: 23
 *        }
 *    }
 */
var bridge = (function (extension) {
    var Callbacks = (function () {
        var _collection = {};
        var _cid = 0;
        var _next = function () {
            return (_cid += 1);
        };

        var CallbackManager = function () {};

        CallbackManager.prototype = {
            add: function (/*callbacks, cid?*/) {
                console.log('bridge', 'CallbackManager', 'add');
                var args = Array.prototype.slice.call(arguments);
                console.dir('bridge', 'CallbackManager', 'add', args);
                var c = args.shift();
                var cid = args.pop();
                if (cid) {
                    if (c !== null && typeof c === 'object') {
                        for (var key in c) {
                            if (c.hasOwnProperty(key)) _collection[cid][key] = c[key];
                        }
                    }
                } else {
                    cid = _next();
                    _collection[cid] = c;
                }
                return cid;
            },
            remove: function (cid) {
                console.log('bridge', 'CallbackManager', 'remove');
                if (_collection[cid]) delete _collection[cid];
            },
            call: function (cid, key, args, keep) {
                console.log('bridge', 'CallbackManager', 'call');
                var callbacks = _collection[cid];
                keep = !!keep;
                if (callbacks) {
                    var fn = callbacks[key];
                    if (fn) {
                        fn.apply(null, args);
                        if (!keep) this.remove(cid)
                    }
                }
            }
        };

        return {
            getInstance: function () {
                return this.instance || (this.instance = new CallbackManager);
            }
        };
    })();


    var Listeners = (function () {
        var _listeners = {};
        var _id = 0;
        var _next = function () {
            return (_id += 1);
        };

        var ListenerManager = function () {};

        ListenerManager.prototype = {
            add: function (l) {
                console.log('bridge', 'ListenerManager', 'add');
                var id = _next();
                _listeners[id] = l;
                return id;
            },
            resolve: function (id, action, data, keep) {
                console.log('bridge', 'ListenerManager', 'resolve');
                keep = !!keep;
                var l = _listeners[id];
                if (l) {
                    var cm = Callbacks.getInstance();
                    cm.call(l.cid, action, [data], keep);
                }
                return l;
            },
            remove: function (id) {
                console.log('bridge', 'ListenerManager', 'remove');
                var l = _listeners[id];
                if (l) {
                    var cm = Callbacks.getInstance();
                    if (l.cid) cm.remove(l.cid);
                    delete _listeners[id];
                }
            }
        }

        return {
            getInstance: function () {
                return this.instance || (this.instance = new ListenerManager);
            }
        };
    })();

    var Listener = function () {
        console.log('bridge', 'Listener constructor');
        this.cid = null;
    };
    Listener.prototype = {
        then: function (c) {
            console.log('bridge', 'Listener', 'then');
            var cm = Callbacks.getInstance();
            this.cid = cm.add(c, this.cid);
            return this;
        }
    };

    var Bridge = function () {};
    Bridge.prototype = {
        sync: function (data) {
            console.log('bridge', 'sync');
            var result = extension.internal.sendSyncMessage(JSON.stringify(data));
            var obj = JSON.parse(result);
            if (obj.error)
                throw new tizen.WebAPIException(obj.code, obj.name, obj.message);
            return obj.result;
        },
        async: function (data) {
            console.log('bridge', 'async');
            var l = new Listener();
            data.cid = Listeners.getInstance().add(l);
            setTimeout(function () {
                extension.postMessage(JSON.stringify(data));
            });
            return l;
        }
    };

    extension.setMessageListener(function (json) {
        /*
         *Expected response:
         *{
         *    cid: 23,                        // callback id
         *    action: 'success',              // expected callback action
         *    keep: false                     // optional param
         *    data: {...}                     // data pased to callback
         *}
         */

        console.log('bridge', 'setMessageListener', json);
        var data = JSON.parse(json);
        if (data.cid && data.action) {
            Listeners.getInstance().resolve(data.cid, data.action, data.data, data.keep);
        }
    });

    return new Bridge;
})(extension);

/**
 * Specifies the Messaging service tags.
 */
var MessageServiceTag = ['messaging.sms', 'messaging.mms', 'messaging.email'];

function Message(type, data) {
    if (!this instanceof Message) {
        return new Message(type, data);
    }
    if (MessageServiceTag.indexOf(type) === -1) {
        throw new tizen.WebAPIException(tizen.WebAPIException.TYPE_MISMATCH_ERR);
    }
    if (data === null || typeof data !== 'object') { // 'data' is optional
        data = {};
    }
    var internal       = data instanceof MessageInit_,
        id             = internal ? data.id             : undefined,
        conversationId = internal ? data.conversationId : undefined,
        folderId       = internal ? data.folderId       : undefined,
        timestamp      = internal ? data.timestamp      : undefined,
        from           = internal ? data.from           : undefined,
        isRead         = internal ? data.isRead         : undefined,
        hasAttachment  = internal ? data.hasAttachment  : undefined,
        inResponseTo   = internal ? data.inResponseTo   : undefined,
        messageStatus  = internal ? data.messageStatus  : undefined,
        attachments    = internal ? data.attachments    : [];
    var body = new MessageBody({messageId: id, plainBody: data.plainBody, htmlBody: data.htmlBody});
    propertyFactory_(this, 'id'            , id                 , Property.E             );
    propertyFactory_(this, 'conversationId', conversationId     , Property.E             );
    propertyFactory_(this, 'folderId'      , folderId           , Property.E             );
    propertyFactory_(this, 'type'          , type               , Property.E             );
    propertyFactory_(this, 'timestamp'     , timestamp          , Property.E             );
    propertyFactory_(this, 'from'          , from               , Property.E             );
    propertyFactory_(this, 'to'            , data.to            , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'cc'            , data.cc            , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'bcc'           , data.bcc           , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'body'          , body               , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'isRead'        , isRead             , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'hasAttachment' , hasAttachment      , Property.E             );
    propertyFactory_(this, 'isHighPriority', data.isHighPriority, Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'subject'       , data.subject       , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'inResponseTo'  , inResponseTo       , Property.E             ); // TODO: setraises
    propertyFactory_(this, 'messageStatus' , messageStatus      , Property.E             );
    propertyFactory_(this, 'attachments'   , attachments        , Property.E | Property.W); // TODO: setraises
};

function MessageInit(data) {
    if (!this instanceof MessageInit) {
        return new MessageInit(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }
    propertyFactory_(this, 'subject'       , data.subject       , Property.E | Property.W);
    propertyFactory_(this, 'to'            , data.to            , Property.E | Property.W);
    propertyFactory_(this, 'cc'            , data.cc            , Property.E | Property.W);
    propertyFactory_(this, 'bcc'           , data.bcc           , Property.E | Property.W);
    propertyFactory_(this, 'plainBody'     , data.plainBody     , Property.E | Property.W);
    propertyFactory_(this, 'htmlBody'      , data.htmlBody      , Property.E | Property.W);
    propertyFactory_(this, 'isHighPriority', data.isHighPriority, Property.E | Property.W);
};

function MessageInit_(data) {
    if (!this instanceof MessageInit_) {
        return new MessageInit_(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }
    this.messaging      = data.messaging;
    this.conversationId = data.conversationId;
    this.folderId       = data.folderId;
    this.timestamp      = data.timestamp;
    this.from           = data.from;
    this.to             = data.to;
    this.cc             = data.cc;
    this.bcc            = data.bcc;
    this.body           = data.body;
    this.isRead         = data.isRead;
    this.hasAttachment  = data.hasAttachment;
    this.isHighPriority = data.isHighPriority;
    this.subject        = data.subject;
    this.inResponseTo   = data.inResponseTo;
    this.messageStatus  = data.messageStatus;
    this.attachments    = data.attachments;
};

function MessageBody(data) {
    if (!this instanceof MessageBody) {
        return new MessageBody(data);
    }
    if (data === null || typeof data !== 'object') {
        data = {};
    }
    propertyFactory_(this, 'messageId'        , data.messageId        , Property.E             );
    propertyFactory_(this, 'loaded'           , data.loaded           , Property.E             );
    propertyFactory_(this, 'plainBody'        , data.plainBody        , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'htmlBody'         , data.htmlBody         , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'inlineAttachments', data.inlineAttachments, Property.E | Property.W); // TODO: setraises
};

function MessageAttachment(filePath, mimeType) {
    propertyFactory_(this, 'id'       , null     , Property.E); // TODO
    propertyFactory_(this, 'messageId', null     , Property.E); // TODO
    propertyFactory_(this, 'mimeType' , mimeType , Property.E);
    propertyFactory_(this, 'filePath' , filePath , Property.E);
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
        data: {
            messageServiceType: args.messageServiceType
        }
    }).then({
        success: function (data) {
            args.successCallback.call(null, new MessageService(data));
        },
        error: function (e) {
            if (args.errorCallback) {
                args.errorCallback.call(
                    null,
                    new tizen.WebAPIException(e.code, e.name, e.message)
                )
            }
        }
    });
}

function MessageService(data) {
    propertyFactory_(this, 'id', data.id, Property.E);
    propertyFactory_(this, 'type', data.type, Property.E);
    propertyFactory_(this, 'name', data.name, Property.E);
    propertyFactory_(this, 'messageStorage', new MessageStorage(data.messageStorage), Property.E);
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
        data: {
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
                    new tizen.WebAPIException(e.code, e.name, e.message)
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
        data: {
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
                    new tizen.WebAPIException(e.code, e.name, e.message)
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
        data: {
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
                    new tizen.WebAPIException(e.code, e.name, e.message)
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

    bridge({
        cmd: 'MessageService_sync',
        data: {
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
                    new tizen.WebAPIException(e.code, e.name, e.message)
                )
            }
        }
    });
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
        data: {
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
                    new tizen.WebAPIException(e.code, e.name, e.message)
                )
            }
        }
    });
};
MessageService.prototype.stopSync = function () {
    var args = validator_.validateArgs(arguments, [
        {name: 'opId', type: types_.LONG}
    ]);

    bridge.sync({
        cmd: 'MessageService_stopSync',
        data: {
            opId: args.opId
        }
    });
};

function MessageStorage() {};

MessageStorage.prototype.addDraftMessage = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'message', type: 'Message'},
        {name: 'successCallback', type: 'function', optional: true, nullable: true},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.findMessages = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'filter', type: 'AbstractFilter'},
        {name: 'successCallback', type: 'function'},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true},
        {name: 'sort', type: 'SortMode', optional: true, nullable: true},
        {name: 'limit', type: 'number', optional: true, nullable: true},
        {name: 'offset', type: 'number', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.removeMessages = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'messages', type: 'array'},
        {name: 'successCallback', type: 'function', optional: true, nullable: true},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.updateMessages = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'messages', type: 'array'},
        {name: 'successCallback', type: 'function', optional: true, nullable: true},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.findConversations = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'filter', type: 'AbstractFilter'},
        {name: 'successCallback', type: 'function'},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true},
        {name: 'sort', type: 'SortMode', optional: true, nullable: true},
        {name: 'limit', type: 'number', optional: true, nullable: true},
        {name: 'offset', type: 'number', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.removeConversations = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'conversations', type: 'array'},
        {name: 'successCallback', type: 'function', optional: true, nullable: true},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.findFolders = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'filter', type: 'AbstractFilter'},
        {name: 'successCallback', type: 'function'},
        {name: 'errorCallback', type: 'function', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.addMessagesChangeListener = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'messagesChangeCallback', type: 'function'},
        {name: 'filter', type: 'AbstractFilter', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype. addConversationsChangeListener = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'conversationsChangeCallback', type: 'function'},
        {name: 'filter', type: 'AbstractFilter', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype. addFoldersChangeListener  = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'conversationsChangeCallback', type: 'function'},
        {name: 'filter', type: 'AbstractFilter', optional: true, nullable: true}
    ]);
    // TODO: STUB
};

MessageStorage.prototype.removeChangeListener = function () {
    var args = xwalk.utils.validateArguments(arguments, [
        {name: 'watchId', type: 'number'}
    ]);
    // TODO: STUB
};

function MessageConversation(data) {
    propertyFactory_(this, 'id'            , data.id            , Property.E);
    propertyFactory_(this, 'type'          , data.type          , Property.E);
    propertyFactory_(this, 'timestamp'     , data.timestamp     , Property.E);
    propertyFactory_(this, 'messageCount'  , data.messageCount  , Property.E);
    propertyFactory_(this, 'unreadMessages', data.unreadMessages, Property.E);
    propertyFactory_(this, 'preview'       , data.preview       , Property.E);
    propertyFactory_(this, 'subject'       , data.subject       , Property.E);
    propertyFactory_(this, 'isRead'        , data.isRead        , Property.E);
    propertyFactory_(this, 'from'          , data.from          , Property.E);
    propertyFactory_(this, 'to'            , data.to            , Property.E);
    propertyFactory_(this, 'cc'            , data.cc            , Property.E);
    propertyFactory_(this, 'bcc'           , data.bcc           , Property.E);
    propertyFactory_(this, 'lastMessageId' , data.lastMessageId , Property.E);
};

function MessageFolder(data) {
    propertyFactory_(this, 'id'            , data.id            , Property.E             );
    propertyFactory_(this, 'parentId'      , data.parentId      , Property.E             );
    propertyFactory_(this, 'serviceId'     , data.serviceId     , Property.E             );
    propertyFactory_(this, 'contentType'   , data.contentType   , Property.E             );
    propertyFactory_(this, 'name'          , data.name          , Property.E | Property.W); // TODO: setraises
    propertyFactory_(this, 'path'          , data.path          , Property.E             );
    propertyFactory_(this, 'type'          , data.type          , Property.E             );
    propertyFactory_(this, 'synchronizable', data.synchronizable, Property.E | Property.W); // TODO: setraises
};

tizen.Message = Message;

tizen.MessageAttachment = MessageAttachment;

exports = new Messaging();
