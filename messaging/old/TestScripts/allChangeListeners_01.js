//
// Tizen Web Device API
// Copyright (c) 2013 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/**
 * @file        allChangeListeners_01.js
 */

var prefferedEmailService = "a.jacak.testmail";

function containCaseInsensitive(string, searchString) {
    var lcString = string.toLowerCase();
    var lcSearchString = searchString.toLowerCase();
    return lcString.indexOf(lcSearchString)>=0;
}

var currentService = null;

//=============================================================================
// FOLDERS
//=============================================================================
function listFolders(prefix, folders) {
    for (var i=0; i<folders.length; i++) {
        var folder = folders[i];
        console.log(prefix + " FOLDER[" + i + "] id: "
                + folder.id
                + " name:" + folder.name
                + " serviceid: " + folder.serviceId);
    }
}

var folderChangeCB = {
    foldersupdated: function(folders) {
        console.log(folders.length + " folder(s) updated");
        listFolders("foldersupdated",folders);
    },
    foldersadded: function(folders) {
        console.log(folders.length + " folder(s) added");
        listFolders("foldersadded",folders);
    },
    foldersremoved: function(folders) {
        console.log(folders.length + " folder(s) removed");
        listFolders("foldersremoved",folders);
    }
 };



//=============================================================================
// CONVERSATIONS
//=============================================================================
function listConversations(prefix, conversations) {

    for (var i=0; i<conversations.length; i++) {
        var conversation = conversations[i];
        console.log(prefix + " CONVERSATION[" + i + "] " +
                " id: " + conversation.id +
                " type: " + conversation.type +
                " timestamp: " + conversation.timestamp +
                " messageCount: " + conversation.messageCount +
                " unreadMessages: " + conversation.unreadMessages +
                " preview: " + conversation.preview +
                " subject:" + conversation.subject +
                " isRead: " + conversation.isRead +
                " from: " + conversation.from +
                " lastMessageId: " + conversation.lastMessageId);
    }
}

var conversationChangeCB = {
    conversationsupdated: function(conversations) {
        console.log(conversations.length + " conversation(s) updated");
        listConversations("conversationsupdated", conversations);
    },
    conversationsadded: function(conversations) {
        console.log(conversations.length + " conversation(s) added");
        listConversations("conversationsadded", conversations);
    },
    conversationsremoved: function(conversations) {
        console.log(conversations.length + " conversation(s) removed");
        listConversations("conversationsremoved", conversations);
    }
};

//=============================================================================
// MESSAGES
//=============================================================================
function listMessages(prefix, messages) {
    for (var i=0; i<messages.length; i++) {
        var message = messages[i];
        console.log(prefix + " MSG[" + i + "] status: "
                    + message.messageStatus
                    + " subject:" + message.subject);
    }
}

var messageChangeCB = {
    messagesupdated: function(messages) {
        console.log(messages.length + " message(s) updated");
        listMessages("messagesupdated",messages);
    },
    messagesadded: function(messages) {
       console.log("messagesadded: " + messages.length + " message(s) added");
       listMessages("messagesadded",messages);

    },
    messagesremoved: function(messages) {
        console.log(messages.length + " message(s) removed");
        listMessages("messagesremoved",messages);
    }
};


//=============================================================================
// Sync
//=============================================================================
function serviceSyncSuccessCB() {
    console.log("Synced!");
}

function serviceSyncFailCB() {
    console.log("Sync failed!");
}

//=============================================================================
// Main
//=============================================================================
function servicesListSuccessCB(services) {
    console.log("servicesListSuccessCB: received:" + services.length + " services");

    if(services.length > 0) {

        var chosenServiceIndex = 0;
        for(var i = 0; i < services.length; i++) {
            if(containCaseInsensitive(services[i].name, prefferedEmailService)) {
                chosenServiceIndex = i;
                console.log("servicesListSuccessCB: " +
                        "found service which name contains: " +
                        prefferedEmailService);
                break;
            }
        }
        currentService = services[chosenServiceIndex];
        console.log("servicesListSuccessCB: testing service:" + currentService.name);

        var filter = null;

        currentService.messageStorage.addMessagesChangeListener(messageChangeCB, filter);
        currentService.messageStorage.addFoldersChangeListener(folderChangeCB, filter);
        currentService.messageStorage.addConversationsChangeListener(
                conversationChangeCB, filter);

        currentService.sync(serviceSyncSuccessCB, serviceSyncFailCB, 30);
    } else {
        console.log("servicesListSuccessCB: ERROR: could not find any email service!");
    }
}

tizen.messaging.getMessageServices("messaging.email", servicesListSuccessCB);
